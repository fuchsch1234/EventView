/*
 * Observer.h
 *
 *  Created on: Apr 25, 2020
 *      Author: cf
 */

#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace fuchsch::utilities {

template<typename T> class Observer;
template<typename T> class Observable;

template<typename T>
struct Deleter {
	Deleter(Observable<T> *_observable) : observable{_observable} {}
	Deleter(Deleter &&observable) noexcept = default;
	Deleter& operator=(Deleter &&observable) noexcept = default;
	void operator()(Observer<T> *observer) {
		if (observable) {
			observable->RemoveObserver(observer);
		}
	}
	Observable<T> *observable = nullptr;
};

template<typename T>
class Observable {
public:

	using CancellationToken = std::unique_ptr<Observer<T>, Deleter<T>>;

	[[nodiscard]] CancellationToken AddObserver(Observer<T> *observer) {
		observers.push_back(observer);
		return CancellationToken(observer, Deleter<T>(this));
	}

	void RemoveObserver(Observer<T> *observer) {
		observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
	}

protected:

	void Notify(T const &arg) const noexcept {
		for (auto &observer : observers) {
			(*observer)(arg);
		}
	}

private:


	std::vector<Observer<T> *> observers = {};

};

template<typename T>
class Observer {
public:

	virtual ~Observer() {};
	virtual void operator()(T const &arg) noexcept = 0;

	void AttachTo(Observable<T> &observable) {
		token = std::move(observable.AddObserver(this));
	}

private:

	typename Observable<T>::CancellationToken token{ nullptr, Deleter<T>(nullptr) };

};

template<typename T, typename O>
inline O& operator>>(Observable<T> &observable, O &observer) {
	observer.AttachTo(observable);
	return observer;
}

} // namespace fuchsch::utilities
