#include <cstdlib>
#include <iostream>
#include <boost\aligned_storage.hpp>
#include <boost\array.hpp>
#include <boost\bind.hpp>
#include <boost\enable_shared_from_this.hpp>
#include <boost\noncopyable.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\asio.hpp>

using boost::asio::ip::tcp;


class handler_allocator
	:private boost::noncopyable
{
public:
	handler_allocator()
		:in_use_(false){}

	void* allocate(std::size_t size)
	{
		if (!in_use_ && size < storage_.size)
		{
			in_use_ = true;
			return storage_.address();
		}
		else{
			return ::operator new(size);
		}
	}

	void deallocate(void* pointer)
	{
		if (pointer == storage_.address())
		{
			in_use_ = false;
		}
		else{
			::operator delete(pointer);
		}
	}

private:
	boost::aligned_storage<1024> storage_;
	bool in_use_;
};

template<typename Handler>
class custom_alloc_handler
{
public:
	custom_alloc_handler(handler_allocator& a, Handler h)
		:allocator_(a), handler_(h){}
	
	template<typename Arg1>
	void operator()(Arg1 arg1)
	{
		handler_(arg1);
	}

	template<typename Arg1, typename Arg2>
	void operator()(Arg1 arg1, Arg2 arg2)
	{
		handler_(arg1, arg2);
	}

	friend void* asio_handler_allocate(std::size_t size,
		custom_alloc_handler<Handler>* this_handler)
	{
		return this_handler->allocator_.allocate(size);
	}

	friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/,
		custom_alloc_handler<Handler>* this_handler)
	{
		this_handler->allocator_.deallocate(pointer);
	}

private:
	handler_allocator& allocator_;
	Handler handler_;
};

template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
	handler_allocator& a, Handler h)
{
	return custom_alloc_handler<Handler>(a, h);
}

class session
	:public boost::enable_shared_from_this < session >
{
public:
	session(boost::asio::io_service& io_service)
		:socket_(io_service)
	{
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_),
			make_custom_alloc_handler(allocator_,
			boost::bind(&session::handle_read,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
	}

	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
	{
		if (!error)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(data_, bytes_transferred),
				make_custom_alloc_handler(allocator_,
				boost::bind(&session::handle_write,
				shared_from_this(),
				boost::asio::placeholders::error)));
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_.async_read_some(boost::asio::buffer(data_),
				make_custom_alloc_handler(allocator_,
				boost::bind(&session::handle_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred)));
		}
	}

private:
	tcp::socket socket_;
	boost::array<char, 1024> data_;
	handler_allocator allocator_;
};

typedef boost::shared_ptr<session> session_ptr;

class server
{
public:
	server(boost::asio::io_service& io_service, short port)
		: io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		session_ptr new_session(new session(io_service_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept(session_ptr new_session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			new_session->start();
		}

		new_session.reset(new session(io_service_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;

		using namespace std;
		server s(io_service, atoi(argv[1]));

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

