#include <boost\bind.hpp>
#include <boost\function.hpp>
#include <boost\thread.hpp>

class button
{
public:
	boost::function<void()> onClick;
};

class player
{
public:
	inline void play(){
		printf("play! \n");
	}
	inline void stop(){
		printf("stoped ! \n");
	}
};

boost::thread make_thread();


class status {
	std::string name_;
	bool ok_;
public:
	status(const std::string& name) :name_(name), ok_(true) {}

	void break_it() {
		ok_ = false;
	}

	bool is_broken() const {
		return ok_;
	}

	void report() const {
		std::cout << name_ << " is " <<
			(ok_ ? "working nominally" : "terribly broken") << '\n';
	}
};

int main()
{
	button playButton, stopButton;
	player thePlayer;

	playButton.onClick = boost::bind(&player::play, &thePlayer);
	stopButton.onClick = boost::bind(&player::stop, &thePlayer);

	
	//boost::thread workthread(boost::bind(&player::play, &thePlayer));
	//boost::thread workthread2(boost::bind(&player::stop, &thePlayer));

	boost::thread_group threadpool;
	threadpool.create_thread(boost::bind(&player::play, &thePlayer));
	threadpool.create_thread(boost::bind(&player::stop, &thePlayer));

	threadpool.join_all();
	
	std::vector<status> statuses;
	statuses.push_back(status("status 1"));
	statuses.push_back(status("status 2"));
	statuses.push_back(status("status 3"));
	statuses.push_back(status("status 4"));

	statuses[1].break_it();
	statuses[2].break_it();

	std::for_each(statuses.begin(), statuses.end(), boost::bind(&status::report, _1));

	return 0;
}