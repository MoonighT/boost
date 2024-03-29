#include "server.hpp"
#include <boost\bind.hpp>
#include <signal.h>

namespace http {
	namespace server {
		server::server(const std::string& address, const std::string& port,
			const std::string& doc_root)
			: io_service_()
			, signals_(io_service_)
			, acceptor_(io_service_)
			, connection_manager_()
			, new_connection_()
			, request_handler_(doc_root)
		{
			signals_.add(SIGINT);
			signals_.add(SIGTERM);
#if defined(SIGQUIT)
			signals_.add(SIGQUIT);
#endif
			signals_.async_wait(boost::bind(&server::handle_stop, this));

			boost::asio::ip::tcp::resolver resolver(io_service_);
			boost::asio::ip::tcp::resolver::query query(address, port);
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
			acceptor_.open(endpoint.protocol());
			acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			acceptor_.bind(endpoint);
			acceptor_.listen();

			start_accept();
		}

		void server::run()
		{
			io_service_.run();
		}

		void server::start_accept()
		{
			new_connection_.reset(new connection(io_service_,
				connection_manager_, request_handler_));
			acceptor_.async_accept(new_connection_->socket(),
				boost::bind(&server::handle_accept, this,
				boost::asio::placeholders::error));
		}

		void server::handle_accept(const boost::system::error_code& e)
		{
			if (!acceptor_.is_open())
			{
				return;
			}
			if (!e)
			{
				connection_manager_.start(new_connection_);
			}
			start_accept();
		}

		void server::handle_stop()
		{
			acceptor_.close();
			connection_manager_.stop_all();
		}

	}
}