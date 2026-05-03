// MIT License
// Copyright (c) 2026 Palaash

#include "Event.h"

#include <deque>

namespace {

std::deque<core::Event> g_eventQueue;

} // namespace

namespace core {

Event::Event() {}

Event::~Event() {}

EventQueue::EventQueue() {}

EventQueue::~EventQueue() {}

void EventQueue::push(const Event& event) {
	g_eventQueue.push_back(event);
}

bool EventQueue::poll(Event& event) {
	if (g_eventQueue.empty()) {
		return false;
	}

	event = g_eventQueue.front();
	g_eventQueue.pop_front();
	return true;
}

void EventQueue::clear() {
	g_eventQueue.clear();
}

std::size_t EventQueue::size() {
	return g_eventQueue.size();
}

} // namespace core