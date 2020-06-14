/*************************************************************************/
/*  tab_container.h                                                      */
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

#ifndef TAB_CONTAINER_H
#define TAB_CONTAINER_H

#include "scene/gui/container.h"
#include "scene/gui/tabs.h"

class TabContainer : public Container {
	GDCLASS(TabContainer, Container);

public:
	enum TabAlign {
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_MAX
	};

private:
	Tabs *tabs;
	bool use_hidden_tabs_for_min_size;

	Vector<Control *> _get_tabs() const;
	int _get_top_margin() const;
	void _on_theme_changed();

protected:
	void _child_renamed_callback();
	void _notification(int p_what);
	virtual void add_child_notify(Node *p_child);
	virtual void remove_child_notify(Node *p_child);

	static void _bind_methods();

public:
	void set_tab_align(TabAlign p_align);
	TabAlign get_tab_align() const;

	void set_tabs_visible(bool p_visible);
	bool are_tabs_visible() const;

	void set_tab_title(int p_tab, const String &p_title);
	String get_tab_title(int p_tab) const;

	void set_tab_icon(int p_tab, const Ref<Texture2D> &p_icon);
	Ref<Texture2D> get_tab_icon(int p_tab) const;

	void set_tab_disabled(int p_tab, bool p_disabled);
	bool get_tab_disabled(int p_tab) const;

	void set_tab_hidden(int p_tab, bool p_hidden);
	bool get_tab_hidden(int p_tab) const;

	int get_tab_count() const;
	void set_current_tab(int p_current);
	int get_current_tab() const;
	int get_previous_tab() const;

	Control *get_tab_control(int p_idx) const;
	Control *get_current_tab_control() const;

	virtual Size2 get_minimum_size() const;

	virtual void get_translatable_strings(List<String> *p_strings) const;

	void set_popup(Node *p_popup);
	Popup *get_popup() const;

	void set_drag_to_rearrange_enabled(bool p_enabled);
	bool get_drag_to_rearrange_enabled() const;
	void set_tabs_rearrange_group(int p_group_id);
	int get_tabs_rearrange_group() const;
	void set_use_hidden_tabs_for_min_size(bool p_use_hidden_tabs);
	bool get_use_hidden_tabs_for_min_size() const;

	TabContainer();
};

VARIANT_ENUM_CAST(TabContainer::TabAlign);

#endif // TAB_CONTAINER_H
