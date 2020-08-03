#pragma once

#include <Button.hpp>
#include <Godot.hpp>
#include <String.hpp>
#include <functional>
#include <iterator>
#include <memory>

namespace godot::structural_inspector {

template <class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

template <class This>
class CloneProvider {
public:
	virtual This* clone() const = 0;

	std::unique_ptr<This> clone_uptr() const {
		return std::unique_ptr<This>(clone());
	}
};

template <class T>
class Iterator {
public:
	virtual bool has_next() const = 0;
	virtual T next() = 0;
};

class NXButton : public Button {
	GODOT_CLASS(NXButton, Button)
private:
	String icon_name;

	void _notification(int what);
	void _apply_icon();

public:
	static void _register_methods();
	void _init();
	void _custom_init(const String& icon_name);

	NXButton();
	~NXButton();
};

String format_variant(const Variant& variant);

} // namespace godot::structural_inspector

template <>
struct std::hash<godot::String> {
	size_t operator()(const godot::String& str) const noexcept;
};
