/*
 * ObserverTest.cpp
 *
 *  Created on: Apr 26, 2020
 *      Author: cf
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fuchsch/Utilities/Observer.h>

class A: public fuchsch::utilities::Observable<int> {
public:

	void Next(int i) {
		Notify(i);
	}

};

class B: public fuchsch::utilities::Observer<int> {
public:

	void operator()(int const &i) noexcept override {
		call(i);
	}

	MOCK_METHOD(void, call, (int));

};

TEST(ObserverTest, ObserverIsCalled) {
	A observable;
	B observer;

	EXPECT_CALL(observer, call(5)).Times(1);

	observer.AttachTo(observable);
	observable.Next(5);
}

TEST(ObserverTest, MultipleObserversAreCalled) {
	A observable;
	B observers[3];

	for (auto &observer: observers) {
		EXPECT_CALL(observer, call(5)).Times(1);
		observer.AttachTo(observable);
	}

	observable.Next(5);
}

TEST(ObserverTest, ObserversRemoveThemselvesOnDestruction) {
	A observable;

	{
		B observers[3];

		for (auto &observer: observers) {
			EXPECT_CALL(observer, call(5)).Times(1);
			observer.AttachTo(observable);
		}
		observable.Next(5);
	}

	observable.Next(4);
}

TEST(ObserverTest, ObserverRemoveOnlyThemselves) {
	A observable;
	B longObserver;
	EXPECT_CALL(longObserver, call(5)).Times(1);
	EXPECT_CALL(longObserver, call(4)).Times(1);
	longObserver.AttachTo(observable);

	{
		B observers[3];

		for (auto &observer: observers) {
			EXPECT_CALL(observer, call(5)).Times(1);
			observer.AttachTo(observable);
		}
		observable.Next(5);
	}

	observable.Next(4);
}

TEST(ObserverTest, ObserverCanBeReattached) {
	A observable, observable2;
	B observer;

	EXPECT_CALL(observer, call(5)).Times(1);
	EXPECT_CALL(observer, call(4)).Times(1);

	observer.AttachTo(observable);
	observable.Next(5);

	observer.AttachTo(observable2);
	observable2.Next(4);
	// Observer should not see this because it is attached to a new observable
	observable.Next(5);
}
