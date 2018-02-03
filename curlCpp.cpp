// curlCpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <cstdlib>
#include <cerrno>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Multi.hpp>
#include <string>
#include "comdef.h"
#include <vector>
#include <thread>
#include <list>
#include <mutex>
#include <atomic>
#include <chrono>


#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#pragma comment(lib, "Ws2_32.lib")
#endif // WIN32


using namespace curlpp::options;
using namespace std;

std::mutex                 mtx;
std::list <curlpp::Easy *> list_easys_free;

std::mutex                 bussy_mtx;
std::list <curlpp::Easy *> list_easys_bussy;

std::atomic <bool>         m_stop = false;


curlpp::Multi requests;

curlpp::Easy * get_free_easy_req(){
	std::unique_lock<std::mutex> lock(mtx);
	if (list_easys_free.size() > 0) {
		curlpp::Easy * req = list_easys_free.front();
		list_easys_free.pop_front();
		return req;
	}
	return nullptr;
}

void push_free_easy_req(curlpp::Easy * req)
{
	assert(req != nullptr);
	std::unique_lock<std::mutex> lock(mtx);
	list_easys_free.push_back(req);
}

curlpp::Easy * get_busy_easy_req()
{
	std::unique_lock<std::mutex> lock(bussy_mtx);
	if (list_easys_bussy.size() > 0) {
		curlpp::Easy * req = list_easys_bussy.front();
		list_easys_bussy.pop_front();
		return req;
	}
	return nullptr;
}

void push_busy_easy_req(curlpp::Easy * req)
{
	assert(req != nullptr);
	std::unique_lock<std::mutex> lock(bussy_mtx);
	list_easys_bussy.push_back(req);
}

void produce()
{
	static int64_t i_counter = 0;
	std::string body = STR_BODY;
	while (!m_stop) {
		curlpp::Easy * request = get_free_easy_req();
		if (request == nullptr) {
			this_thread::sleep_for(chrono::milliseconds(0));
			continue;
		}
		(*request).reset();
		if (i_counter++ % 2 == 0) {
		
			(*request).setOpt(new curlpp::options::Url(URL));
			(*request).setOpt(new curlpp::options::Verbose(false));
			std::list<std::string> header;
			header.push_back("Content-Type: application/octet-stream");
			(*request).setOpt(new curlpp::options::HttpHeader(header));
			(*request).setOpt(new curlpp::options::PostFields(body));
			(*request).setOpt(new curlpp::options::PostFieldSize(body.length()));
			(*request).setOpt(new curlpp::options::SslVerifyPeer(false));
		} else {
			(*request).setOpt(new curlpp::options::Url(GET_URL));
			(*request).setOpt(new curlpp::options::Verbose(false));
			(*request).setOpt(new curlpp::options::SslVerifyPeer(false));
		}
		push_busy_easy_req(request); 
		printf("push_busy_easy_req -- \n");
		this_thread::sleep_for(chrono::milliseconds(50));
	}
	printf("produce finished \n");
}

void consumer()
{
	int nbLeft;
	while (!m_stop) {
		curlpp::Easy * request = get_busy_easy_req();
		if (request == nullptr) {
			this_thread::sleep_for(chrono::milliseconds(0));
			continue;
		}
		requests.add(request);
		while (!requests.perform(&nbLeft)) {};
		break;
	}
	std::vector<curl_waitfd> fds;
	int msg_in_queue;
	static int64_t i_counter = 0;
	while (!m_stop ) {
		int ret = requests.wait(100, fds);
		requests.perform(&nbLeft);
		//if (nbLeft > 0) {
			curlpp::Multi::Msgs msgs = requests.info();
			for (auto pos = msgs.begin(); pos != msgs.end(); pos++) {
				if (pos->second.msg == CURLMSG_DONE) {
					requests.remove(pos->first);
					curlpp::Easy * req = (curlpp::Easy *)(pos->first);
					push_free_easy_req(req);
					int res_code = curlpp::infos::ResponseCode::get(*req);
					printf("Cur request counter= %lld completed with status %d res_code =%d\n", i_counter++, pos->second.code, res_code);

				}
			}
		//}

		curlpp::Easy * request = get_busy_easy_req();
		if (request == nullptr) {
			this_thread::sleep_for(chrono::milliseconds(0));
			continue;
		} else {
			requests.add(request);
			while (!requests.perform(&nbLeft)) {};
		}
	//	Sleep(1000);
		
	}
	printf("consumer finished \n");
}

#if 1
int main(int, char **){

	for (int i = 0; i < 10;i++)
	{
		curlpp::Easy * req = new curlpp::Easy;
		push_free_easy_req(req);
	}
	 
	std::thread produce_t(produce);
	produce_t.detach();

	std::thread consumer_t(consumer);
	consumer_t.detach();

	while (std::getchar() != 'c') {
		
	}
	m_stop = true;

	if (consumer_t.joinable()) consumer_t.join();
	if (produce_t.joinable()) produce_t.join();
}

#endif

#if 0
int main(int, char **)
{


	char *url = URL;
	std::string body = STR_BODY;

	try {
		curlpp::Cleanup cleaner;
		curlpp::Easy request;

		request.setOpt(new curlpp::options::Url(url));
		request.setOpt(new curlpp::options::Verbose(false));
		std::list<std::string> header;
		header.push_back("Content-Type: application/octet-stream");
		request.setOpt(new curlpp::options::HttpHeader(header));
		request.setOpt(new curlpp::options::PostFields(body));
		request.setOpt(new curlpp::options::PostFieldSize(body.length()));
		request.setOpt(new curlpp::options::SslVerifyPeer(false));
		

		curlpp::Easy request1;

		request1.setOpt(new curlpp::options::Url(GET_URL));
		request1.setOpt(new curlpp::options::Verbose(false));
		request1.setOpt(new curlpp::options::SslVerifyPeer(false));

		curlpp::Multi requests;
		requests.add(&request);
		requests.add(&request1);

		int nbLeft;
		/* we start some action by calling perform right away */
		while (!requests.perform(&nbLeft)) {};

		std::vector<curl_waitfd> fds;
		while (nbLeft) {
			int ret = requests.wait(100, fds);
			requests.perform(&nbLeft);
		}

		std::cout << "NB lefts: " << nbLeft << std::endl;

		curlpp::Multi::Msgs msgs = requests.info();
		for (curlpp::Multi::Msgs::iterator pos = msgs.begin();
			pos != msgs.end();
			pos++) {
			if (pos->second.msg == CURLMSG_DONE) {

				/* Find out which handle this message is about */
				if (pos->first == &request1) {
					int res_code = curlpp::infos::ResponseCode::get(request);
						printf("First request completed with status %d res_code =%d\n", pos->second.code, res_code);
				} else if (pos->first == &request) {
					int res_code = curlpp::infos::ResponseCode::get(request);
					printf("Secound request completed with status %d res_code =%d\n", pos->second.code, res_code);
				}
			}
		}
	}
	catch (curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}
	catch (curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
#endif

#if 0


int main(int argc, char *argv[])
{


	char *url1 = "https://www.baidu.com/";
	char *url2 = "https://www.baidu.com/";

	try {
		curlpp::Cleanup cleaner;

		curlpp::Easy request1;
		curlpp::Easy request2;

		request1.setOpt(new curlpp::options::Url(url1));
		request1.setOpt(new curlpp::options::Verbose(true));

		request2.setOpt(new curlpp::options::Url(url2));
		request2.setOpt(new curlpp::options::Verbose(true));

		int nbLeft;
		curlpp::Multi requests;
		requests.add(&request1);
		requests.add(&request2);

		/* we start some action by calling perform right away */
		while (!requests.perform(&nbLeft)) {};

		std::vector<curl_waitfd> fds;
		while (nbLeft) {
			int ret = requests.wait(100, fds);
			requests.perform(&nbLeft);
		}

		std::cout << "NB lefts: " << nbLeft << std::endl;

		/* See how the transfers went */
		/*
		Multi::info returns a list of:
		std::pair< curlpp::Easy, curlpp::Multi::Info >
		*/
		curlpp::Multi::Msgs msgs = requests.info();
		for (curlpp::Multi::Msgs::iterator pos = msgs.begin();
			pos != msgs.end();
			pos++) {
			if (pos->second.msg == CURLMSG_DONE) {

				/* Find out which handle this message is about */
				if (pos->first == &request1) {
					printf("First request completed with status %d\n", pos->second.code);
				} else if (pos->first == &request2) {
					printf("Second request completed with status %d\n", pos->second.code);
				}
			}
		}
	}
	catch (curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}
	catch (curlpp::RuntimeError & e) {
		std::cout << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
#endif