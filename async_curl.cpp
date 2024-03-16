// Async tcp server
// recieve URL, send it to child with curl
// recieve content from curl, send it to client

#include <iostream>
#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace {
using boost::asio::ip::tcp;
namespace bp = boost::process;

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
	tcp::socket socket_;
	std::string url_;
	boost::asio::streambuf buf_;
	std::shared_ptr<bp::child> child_ptr_;

	void do_read() {
		auto self(shared_from_this());
		// read URL
		url_.clear();
		boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(url_), '\n', 
			[this, self](boost::system::error_code ec, std::size_t length) {
				if(!ec) {
					url_.erase(std::find(url_.begin(), url_.end(), '\r'), url_.end());
					std::cerr << "URL: " << url_ << std::endl;
					read_from_child(length);
				}
			});
	}
	void read_from_child(std::size_t length) {
		auto self(shared_from_this());
		// run curl
		child_ptr_ = std::make_shared<bp::child>(bp::search_path("curl"), url_,
			bp::std_out > buf_,
		    	bp::std_err > buf_,
		  	(boost::asio::io_context&)socket_.get_executor().context(),
			bp::on_exit = [this, self](int e, const std::error_code& ec) {
				std::cerr << "child exited with " << e << " " << ec.message() << std::endl;
				do_write();
			}
		);
	}

	void do_write() {
		auto self(shared_from_this());
		// write to client
		async_write(socket_, buf_, 
			[this, self](boost::system::error_code ec, std::size_t length) {
				if(!ec) {
					std::cerr << "send " << length << std::endl;
					do_read();
				}
			});
	}
public:
	explicit tcp_connection(tcp::socket socket)
		: socket_(std::move(socket)) {
	}
	~tcp_connection() {
	}

	void start() {
		do_read();
	}
};

class tcp_server {
	tcp::acceptor acceptor_;

	void start_accept() {
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket) {
				if(!ec) {
					std::make_shared<tcp_connection>(std::move(socket))->start();
				}
				start_accept();
			});
	}
public:
	explicit tcp_server(boost::asio::io_context& io, int port)
		: acceptor_(io, tcp::endpoint(tcp::v4(), port)) // listem
	{
		start_accept();
	}
	~tcp_server() {
	}
};
} // namespace

int main (int argc, char** argv) {
	try {
		if(argc < 2) {
			std::cerr << "Usage: " << basename(argv[0]) << " port\n";
			return -1;
		}
		boost::asio::io_context ioc;
		tcp_server s(ioc, std::atoi(argv[1]));
		ioc.run();
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}
	catch (...) {
		std::cerr << "Unknown error\n";
		return -1;
	}
	return 0;
}
