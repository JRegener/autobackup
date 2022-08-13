#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

using namespace std::chrono_literals;

class Timer {
private:
	using days = std::chrono::duration<int, std::ratio<86400>>;

	using handler_t = std::function<void ()>;
public:
	enum class Mode {
		For,
		Until
	};

	explicit Timer (const handler_t& handler, Mode mode, std::chrono::seconds interval, bool repeat = false)
		: handler (handler), mode (mode), interval (interval), repeat (repeat), stopped (true)
	{
		start ();
	}


	~Timer () { stop (); }

	void start () {
		stop ();

		{
			std::unique_lock<std::mutex> (mutex);
			stopped = false;
		}

		thread = std::thread ([&]() {
			std::unique_lock<std::mutex> lock (mutex);

			while (!stopped) {
				std::cv_status status;

				if (mode == Mode::For) {
					status = cv.wait_for (lock, interval);
				}
				else {
					auto now = std::chrono::system_clock::now ();
					auto day = std::chrono::duration_cast<days>(now.time_since_epoch ());

					auto offset = std::chrono::current_zone ()->get_info (now).offset;

					auto destination = day + interval - offset;
					if (destination < std::chrono::duration_cast<std::chrono::seconds> (now.time_since_epoch ()))
						destination += days{ 1 };
					status = cv.wait_until (lock, std::chrono::system_clock::time_point (destination));
				}

				if (status == std::cv_status::timeout) {
					if (!repeat) stopped = true;
					handler ();
					if (mode == Mode::Until) std::this_thread::sleep_for (1s);
				}
			}
			});
	}

	void stop () {
		{
			std::unique_lock<std::mutex> (mutex);
			if (stopped) return;
			stopped = true;
		}

		cv.notify_one ();

		if (thread.joinable ()) {
			thread.join ();
		}
	}

	bool running () noexcept {
		std::unique_lock<std::mutex> (mutex);
		return !stopped;
	}

private:
	std::chrono::seconds interval;
	const Mode mode;
	bool repeat;
	bool stopped;
	const handler_t handler;
	std::thread thread;
	std::mutex mutex;
	std::condition_variable cv;
};
