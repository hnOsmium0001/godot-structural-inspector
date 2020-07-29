#pragma once

#include <iterator>

namespace godot::structural_editor {

template <class T>
class Iterator {
public:
	virtual bool has_next() const = 0;
	virtual T next() = 0;
};

// template <class StdIter>
// class StdBackedIterator : public Iterator<typename std::iterator_traits<StdIter>::reference_type> {
// private:
// 	StdIter begin;
// 	StdIter end;

// public:
// 	StdBackedIterator(StdIter begin, StdIter end) :
// 			begin{ begin }, end{ end } {}
	
// 	bool has_next() const override {
// 		return begin != end;
// 	}

// 	T next() override {
// 		auto& val = *begin;
// 		++begin;
// 		return val;
// 	}
// };

} // namespace godot::structural_editor
