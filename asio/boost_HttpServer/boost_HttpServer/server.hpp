#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include <boost\asio.hpp>
#include <boost\noncopyable.hpp>
#include "connection_manager.hpp"

namespace http{
	namespace server {
		class server
			: private boost::noncopyable
		{
		public:
			explicit server(const std::string& address, const std::string& port,
				const std::string& doc_root);

			void run();
		private:
			void start_accept();
			void handle_accept(const boost::system::error_code& e);
			void handle_stop();

		private:
			boost::asio::io_service io_service_;
			boost::asio::signal_set signals_;
			boost::asio::ip::tcp::acceptor acceptor_;
			connection_manager connection_manager_;
			connection_ptr new_connection_;
			request_handler request_handler_;
		};

	}
}

#endif