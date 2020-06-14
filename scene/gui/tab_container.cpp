/*************************************************************************/
/*  tab_container.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "tab_container.h"

#include "core/message_queue.h"
#include "scene/gui/box_container.h"
#include "scene/gui/label.h"
#include "scene/gui/texture_rect.h"

int TabContainer::_get_top_margin() const {
	if (!are_tabs_visible()) {
		return 0;
	}
	return tabs->get_size().height;

	// // Respect the minimum tab height.
	// Ref<StyleBox> tab_bg = get_theme_stylebox("tab_bg");
	// Ref<StyleBox> tab_fg = get_theme_stylebox("tab_fg");
	// Ref<StyleBox> tab_disabled = get_theme_stylebox("tab_disabled");

	// int tab_height = MAX(MAX(tab_bg->get_minimum_size().height, tab_fg->get_minimum_size().height), tab_disabled->get_minimum_size().height);

	// // Font height or higher icon wins.
	// Ref<Font> font = get_theme_font("font");
	// int content_height = font->get_height();

	// Vector<Control *> tabcs = _get_tabs();
	// for (int i = 0; i < tabcs.size(); i++) {
	// 	Control *c = tabcs[i];
	// 	if (!c->has_meta("_tab_icon")) {
	// 		continue;
	// 	}

	// 	Ref<Texture2D> tex = c->get_meta("_tab_icon");
	// 	if (!tex.is_valid()) {
	// 		continue;
	// 	}
	// 	content_height = MAX(content_height, tex->get_size().height);
	// }

	// return tab_height + content_height;
}

void TabContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSLATION_CHANGED: {
			minimum_size_changed();
			update();
		} break;
		case NOTIFICATION_DRAW: {
			RID canvas = get_canvas_item();
			Size2 size = get_size();

			// Draw only the tab area if the header is hidden.
			Ref<StyleBox> panel = get_theme_stylebox("panel");
			if (!are_tabs_visible()) {
				panel->draw(canvas, Rect2(0, 0, size.width, size.height));
				return;
			}
			// Draw the tab area.
			// panel->draw(canvas, Rect2(0, header_height, size.width, size.height - header_height));

		} break;
		case NOTIFICATION_THEME_CHANGED: {
			minimum_size_changed();
			// todo update tabs theme?
			call_deferred("_on_theme_changed"); // Wait until all changed theme.
		} break;
	}
}

void TabContainer::_on_theme_changed() {
	if (get_tab_count() > 0) {
		set_current_tab(get_current_tab());
	}
}

Vector<Control *> TabContainer::_get_tabs() const {
	Vector<Control *> controls;
	// the first child is our Tabs, ignore it
	//todo  might not be possible, actually. others modify our children to do anything
	// todo so instead inherit from Tabs
	// todo or possibly overwrite all Node functions that deal with modifying children, so no one else can touch it

	for (int i = 1; i < get_child_count(); i++) {
		Control *control = Object::cast_to<Control>(get_child(i));
		if (!control || control->is_toplevel_control()) {
			continue;
		}

		controls.push_back(control);
	}
	return controls;
}

void TabContainer::_child_renamed_callback() {
	// todo update the title
	update();
}

void TabContainer::add_child_notify(Node *p_child) {
	Container::add_child_notify(p_child);

	Control *c = Object::cast_to<Control>(p_child);
	if (!c) {
		return;
	}
	if (c->is_set_as_toplevel()) {
		return;
	}
	if (c == tabs || get_child_count() == 1) {
		return;
	}

	tabs->add_tab(c->get_name());

	bool first = false;

	if (get_tab_count() != 1) {
		c->hide();
	} else {
		c->show();
		//call_deferred("set_current_tab",0);
		first = true;
		tabs->set_current_tab(0);
	}
	c->set_anchors_and_margins_preset(Control::PRESET_WIDE);
	if (are_tabs_visible()) {
		c->set_margin(MARGIN_TOP, _get_top_margin());
	}
	Ref<StyleBox> sb = get_theme_stylebox("panel");
	c->set_margin(Margin(MARGIN_TOP), c->get_margin(Margin(MARGIN_TOP)) + sb->get_margin(Margin(MARGIN_TOP)));
	c->set_margin(Margin(MARGIN_LEFT), c->get_margin(Margin(MARGIN_LEFT)) + sb->get_margin(Margin(MARGIN_LEFT)));
	c->set_margin(Margin(MARGIN_RIGHT), c->get_margin(Margin(MARGIN_RIGHT)) - sb->get_margin(Margin(MARGIN_RIGHT)));
	c->set_margin(Margin(MARGIN_BOTTOM), c->get_margin(Margin(MARGIN_BOTTOM)) - sb->get_margin(Margin(MARGIN_BOTTOM)));

	update();
	p_child->connect("renamed", callable_mp(this, &TabContainer::_child_renamed_callback));

	if (first && is_inside_tree()) {
		emit_signal("tab_changed", get_current_tab());
	}
}

int TabContainer::get_tab_count() const {
	return tabs->get_tab_count();
}

void TabContainer::set_current_tab(int p_current) {
	ERR_FAIL_INDEX(p_current, get_tab_count());

	int pending_previous = get_current_tab();
	tabs->set_current_tab(p_current);

	Ref<StyleBox> sb = get_theme_stylebox("panel");
	Vector<Control *> tabcs = _get_tabs();
	for (int i = 0; i < tabcs.size(); i++) {
		Control *c = tabcs[i];
		if (i == p_current) {
			c->show();
			c->set_anchors_and_margins_preset(Control::PRESET_WIDE);
			if (tabs->are_tabs_visible()) {
				c->set_margin(MARGIN_TOP, _get_top_margin());
			}
			c->set_margin(Margin(MARGIN_TOP), c->get_margin(Margin(MARGIN_TOP)) + sb->get_margin(Margin(MARGIN_TOP)));
			c->set_margin(Margin(MARGIN_LEFT), c->get_margin(Margin(MARGIN_LEFT)) + sb->get_margin(Margin(MARGIN_LEFT)));
			c->set_margin(Margin(MARGIN_RIGHT), c->get_margin(Margin(MARGIN_RIGHT)) - sb->get_margin(Margin(MARGIN_RIGHT)));
			c->set_margin(Margin(MARGIN_BOTTOM), c->get_margin(Margin(MARGIN_BOTTOM)) - sb->get_margin(Margin(MARGIN_BOTTOM)));

		} else {
			c->hide();
		}
	}

	// todo reroute signals
	// and remove current and previous
	_change_notify("current_tab");

	if (pending_previous == p_current) {
		emit_signal("tab_selected", p_current);
	} else {
		// previous = pending_previous;
		emit_signal("tab_selected", p_current);
		emit_signal("tab_changed", p_current);
	}

	update();
}

int TabContainer::get_current_tab() const {
	return tabs->get_current_tab();
}

int TabContainer::get_previous_tab() const {
	return tabs->get_previous_tab();
}

Control *TabContainer::get_tab_control(int p_idx) const {
	Vector<Control *> tabcs = _get_tabs();
	if (p_idx >= 0 && p_idx < tabcs.size()) {
		return tabcs[p_idx];
	} else {
		return nullptr;
	}
}

Control *TabContainer::get_current_tab_control() const {
	return get_tab_control(get_current_tab());
}

void TabContainer::remove_child_notify(Node *p_child) {
	// get the index
	int idx = -1;
	if (p_child->get_index() >= 0 && p_child->get_index() < get_child_count()) {
		if (get_child(p_child->get_index()) == p_child) {
			idx = p_child->get_index() - 1;
		}
	}
	if (idx == -1) { //maybe removed while unparenting or something and index was not updated, so just in case the above fails, try this.
		for (int i = 0; i < get_child_count(); i++) {
			if (get_child(i) == p_child) {
				idx = i - 1;
				break;
			}
		}
	}

	Container::remove_child_notify(p_child);

	p_child->disconnect("renamed", callable_mp(this, &TabContainer::_child_renamed_callback));

	tabs->remove_tab(idx);
	// if (idx != -1) {
	// }

	update();
}

void TabContainer::set_tab_align(TabContainer::TabAlign p_align) {
	ERR_FAIL_INDEX(p_align, ALIGN_MAX);
	Tabs::TabAlign tab_align = (Tabs::TabAlign)p_align;
	tabs->set_tab_align(tab_align);
	update();

	_change_notify("tab_align");
}

TabContainer::TabAlign TabContainer::get_tab_align() const {
	return (TabContainer::TabAlign)(tabs->get_tab_align());
}

void TabContainer::set_tabs_visible(bool p_visible) {
	// if (p_visible == tabs->are_tabs_visible()) {
	// 	return;
	// }

	tabs->set_tabs_visible(p_visible);

	Vector<Control *> tabcs = _get_tabs();
	for (int i = 0; i < tabcs.size(); i++) {
		Control *c = tabcs[i];
		if (p_visible) {
			c->set_margin(MARGIN_TOP, _get_top_margin());
		} else {
			c->set_margin(MARGIN_TOP, 0);
		}
	}

	update();
	minimum_size_changed();
}

bool TabContainer::are_tabs_visible() const {
	return tabs->are_tabs_visible();
}

void TabContainer::set_tab_title(int p_tab, const String &p_title) {
	tabs->set_tab_title(p_tab, p_title);
}

String TabContainer::get_tab_title(int p_tab) const {
	return tabs->get_tab_title(p_tab);
}

void TabContainer::set_tab_icon(int p_tab, const Ref<Texture2D> &p_icon) {
	tabs->set_tab_icon(p_tab, p_icon);
}

Ref<Texture2D> TabContainer::get_tab_icon(int p_tab) const {
	return tabs->get_tab_icon(p_tab);
}

void TabContainer::set_tab_disabled(int p_tab, bool p_disabled) {
	tabs->set_tab_disabled(p_tab, p_disabled);
}

bool TabContainer::get_tab_disabled(int p_tab) const {
	return tabs->get_tab_disabled(p_tab);
}

void TabContainer::set_tab_hidden(int p_tab, bool p_hidden) {
	tabs->set_tab_hidden(p_tab, p_hidden);
	if (get_current_tab() == p_tab) {
		// tabs should have changed current but if it can't then
		// no other tab can be switched to, just hide
		get_tab_control(p_tab)->hide();
	}
	update();
}

bool TabContainer::get_tab_hidden(int p_tab) const {
	return tabs->get_tab_hidden(p_tab);
}

// todo test
void TabContainer::get_translatable_strings(List<String> *p_strings) const {
	for (int i = 0; i < tabs->get_tab_count(); i++) {
		String name = tabs->get_tab_title(i);
		if (name != "") {
			p_strings->push_back(name);
		}
	}
}

Size2 TabContainer::get_minimum_size() const {
	Size2 ms;

	Vector<Control *> tabcs = _get_tabs();
	for (int i = 0; i < tabcs.size(); i++) {
		Control *c = tabcs[i];

		if (!c->is_visible_in_tree() && !use_hidden_tabs_for_min_size) {
			continue;
		}

		Size2 cms = c->get_combined_minimum_size();
		ms.x = MAX(ms.x, cms.x);
		ms.y = MAX(ms.y, cms.y);
	}

	// todo use tabs min size?
	Ref<StyleBox> tab_bg = get_theme_stylebox("tab_bg");
	Ref<StyleBox> tab_fg = get_theme_stylebox("tab_fg");
	Ref<StyleBox> tab_disabled = get_theme_stylebox("tab_disabled");
	Ref<Font> font = get_theme_font("font");

	if (are_tabs_visible()) {
		ms.y += MAX(MAX(tab_bg->get_minimum_size().y, tab_fg->get_minimum_size().y), tab_disabled->get_minimum_size().y);
		ms.y += font->get_height();
	}

	Ref<StyleBox> sb = get_theme_stylebox("panel");
	ms += sb->get_minimum_size();

	return ms;
}

void TabContainer::set_popup(Node *p_popup) {
	tabs->set_popup(p_popup);
}

Popup *TabContainer::get_popup() const {
	return tabs->get_popup();
}

void TabContainer::set_drag_to_rearrange_enabled(bool p_enabled) {
	tabs->set_drag_to_rearrange_enabled(p_enabled);
}

bool TabContainer::get_drag_to_rearrange_enabled() const {
	return tabs->get_drag_to_rearrange_enabled();
}

void TabContainer::set_tabs_rearrange_group(int p_group_id) {
	tabs->set_tabs_rearrange_group(p_group_id);
}

int TabContainer::get_tabs_rearrange_group() const {
	return tabs->get_tabs_rearrange_group();
}

void TabContainer::set_use_hidden_tabs_for_min_size(bool p_use_hidden_tabs) {
	use_hidden_tabs_for_min_size = p_use_hidden_tabs;
}

bool TabContainer::get_use_hidden_tabs_for_min_size() const {
	return use_hidden_tabs_for_min_size;
}

void TabContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_tab_count"), &TabContainer::get_tab_count);
	ClassDB::bind_method(D_METHOD("set_current_tab", "tab_idx"), &TabContainer::set_current_tab);
	ClassDB::bind_method(D_METHOD("get_current_tab"), &TabContainer::get_current_tab);
	ClassDB::bind_method(D_METHOD("get_previous_tab"), &TabContainer::get_previous_tab);
	ClassDB::bind_method(D_METHOD("get_current_tab_control"), &TabContainer::get_current_tab_control);
	ClassDB::bind_method(D_METHOD("get_tab_control", "tab_idx"), &TabContainer::get_tab_control);
	ClassDB::bind_method(D_METHOD("set_tab_align", "align"), &TabContainer::set_tab_align);
	ClassDB::bind_method(D_METHOD("get_tab_align"), &TabContainer::get_tab_align);
	ClassDB::bind_method(D_METHOD("set_tabs_visible", "visible"), &TabContainer::set_tabs_visible);
	ClassDB::bind_method(D_METHOD("are_tabs_visible"), &TabContainer::are_tabs_visible);
	ClassDB::bind_method(D_METHOD("set_tab_title", "tab_idx", "title"), &TabContainer::set_tab_title);
	ClassDB::bind_method(D_METHOD("get_tab_title", "tab_idx"), &TabContainer::get_tab_title);
	ClassDB::bind_method(D_METHOD("set_tab_icon", "tab_idx", "icon"), &TabContainer::set_tab_icon);
	ClassDB::bind_method(D_METHOD("get_tab_icon", "tab_idx"), &TabContainer::get_tab_icon);
	ClassDB::bind_method(D_METHOD("set_tab_disabled", "tab_idx", "disabled"), &TabContainer::set_tab_disabled);
	ClassDB::bind_method(D_METHOD("get_tab_disabled", "tab_idx"), &TabContainer::get_tab_disabled);
	ClassDB::bind_method(D_METHOD("set_popup", "popup"), &TabContainer::set_popup);
	ClassDB::bind_method(D_METHOD("get_popup"), &TabContainer::get_popup);
	ClassDB::bind_method(D_METHOD("set_drag_to_rearrange_enabled", "enabled"), &TabContainer::set_drag_to_rearrange_enabled);
	ClassDB::bind_method(D_METHOD("get_drag_to_rearrange_enabled"), &TabContainer::get_drag_to_rearrange_enabled);
	ClassDB::bind_method(D_METHOD("set_tabs_rearrange_group", "group_id"), &TabContainer::set_tabs_rearrange_group);
	ClassDB::bind_method(D_METHOD("get_tabs_rearrange_group"), &TabContainer::get_tabs_rearrange_group);
	ClassDB::bind_method(D_METHOD("set_use_hidden_tabs_for_min_size", "enabled"), &TabContainer::set_use_hidden_tabs_for_min_size);
	ClassDB::bind_method(D_METHOD("get_use_hidden_tabs_for_min_size"), &TabContainer::get_use_hidden_tabs_for_min_size);

	ClassDB::bind_method(D_METHOD("_on_theme_changed"), &TabContainer::_on_theme_changed);

	ADD_SIGNAL(MethodInfo("tab_changed", PropertyInfo(Variant::INT, "tab")));
	ADD_SIGNAL(MethodInfo("tab_selected", PropertyInfo(Variant::INT, "tab")));
	ADD_SIGNAL(MethodInfo("pre_popup_pressed"));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "tab_align", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_tab_align", "get_tab_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "current_tab", PROPERTY_HINT_RANGE, "-1,4096,1", PROPERTY_USAGE_EDITOR), "set_current_tab", "get_current_tab");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tabs_visible"), "set_tabs_visible", "are_tabs_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "drag_to_rearrange_enabled"), "set_drag_to_rearrange_enabled", "get_drag_to_rearrange_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_hidden_tabs_for_min_size"), "set_use_hidden_tabs_for_min_size", "get_use_hidden_tabs_for_min_size");

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);
	BIND_ENUM_CONSTANT(ALIGN_MAX);
}

TabContainer::TabContainer() {
	tabs = memnew(Tabs);
	add_child(tabs);
	// todo connect some signals?

	use_hidden_tabs_for_min_size = false;

}
