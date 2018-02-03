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



#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#pragma comment(lib, "Ws2_32.lib")
#endif // WIN32


using namespace curlpp::options;

#if 1
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

		request1.setOpt(new curlpp::options::Url(url));
		request1.setOpt(new curlpp::options::Verbose(true));

		request1.setOpt(new curlpp::options::PostFields(body));
		request1.setOpt(new curlpp::options::PostFieldSize(body.length()));
		request1.setOpt(new curlpp::options::SslVerifyPeer(false));

		curlpp::Multi requests;
		requests.add(&request);
	//	requests.add(&request1);

		int nbLeft;
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