#include <iostream>
#include <string>

#include <boost\asio.hpp>
#include <boost\array.hpp>
#include <boost\thread.hpp>


boost::asio::io_service io_service_;
boost::asio::ip::tcp::resolver resolver_(io_service_);
boost::asio::ip::tcp::socket socket_(io_service_);
boost::array<char, 4096> buffer_;


void read_handler(const boost::system::error_code &ec, std::size_t byte_transferred)
{
	static int i = 0;
	if (!ec)
	{
		//std::cout << ++i << std::endl;
		std::cout << std::string(buffer_.data(), byte_transferred) << std::endl;
		socket_.async_read_some(boost::asio::buffer(buffer_), read_handler);
	}
}

void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{

}

void connect_handler(const boost::system::error_code &ec)
{
	if (!ec)
	{
		for (int i = 0; i < 200; ++i){
			boost::asio::async_write(socket_, boost::asio::buffer("GET / HTTP 1.1\r\nHost: highscore.de\r\n\r\n"), boost::asio::transfer_at_least(45), write_handler);
			//socket_.async_read_some(boost::asio::buffer(buffer_), read_handler);
		}
	}
}

void resolve_handler(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it)
{
	if (!ec)
	{
		socket_.async_connect(*it, connect_handler);
	}
}

void query(std::string address)
{
	boost::asio::ip::tcp::resolver::query query(address, "80");
	resolver_.async_resolve(query, resolve_handler);
}

void run()
{
	io_service_.run();
}

int main()
{
	query("www.highscore.de");
	
	boost::thread thread1(run);
	boost::thread thread2(run);
	boost::thread thread3(run);
	boost::thread thread4(run);

	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	return 0;
}