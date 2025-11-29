/**
 * @file selector.cpp
 * @brief Autonomous Selector Implementation with LVGL UI
 */

#include "auton/selector.hpp"
#include "host/ipc.hpp"
#include <iostream>

namespace auton {

// Singleton instance
Selector& Selector::instance() {
    static Selector instance;
    return instance;
}

Selector::Selector()
    : _initialized(false),
      _tabview(nullptr),
      _match_tab(nullptr),
      _skills_tab(nullptr),
      _match_label(nullptr),
      _skills_label(nullptr),
      _selected_match(-1),
      _selected_skills(-1) {}

Selector::~Selector() {
    destroy();
}

void Selector::init() {
    if (_initialized) return;
    
    create_ui();
    _initialized = true;
    
    std::cout << "Autonomous selector initialized" << std::endl;
}

void Selector::destroy() {
    if (!_initialized) return;
    
    // Clean up buttons
    for (auto btn : _match_buttons) {
        lv_obj_del(btn);
    }
    _match_buttons.clear();
    
    for (auto btn : _skills_buttons) {
        lv_obj_del(btn);
    }
    _skills_buttons.clear();
    
    // Clean up tabview
    if (_tabview) {
        lv_obj_del(_tabview);
        _tabview = nullptr;
    }
    
    _initialized = false;
}

bool Selector::is_initialized() {
    return _initialized;
}

void Selector::create_ui() {
    // Get active screen
    lv_obj_t* scr = lv_scr_act();
    
    // Create tabview with tabs on top, 50px tab height
    _tabview = lv_tabview_create(scr, LV_DIR_TOP, 50);
    lv_obj_set_size(_tabview, 480, 272);
    lv_obj_set_pos(_tabview, 0, 0);
    
    // Add tabs
    _match_tab = lv_tabview_add_tab(_tabview, "Match");
    _skills_tab = lv_tabview_add_tab(_tabview, "Skills");
    
    // Create description labels
    _match_label = lv_label_create(_match_tab);
    lv_obj_align(_match_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(_match_label, "Select a match autonomous");
    
    _skills_label = lv_label_create(_skills_tab);
    lv_obj_align(_skills_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_text(_skills_label, "Select a skills autonomous");
    
    // Populate buttons
    update_buttons();
}

void Selector::update_buttons() {
    // Clear existing buttons
    for (auto btn : _match_buttons) {
        lv_obj_del(btn);
    }
    _match_buttons.clear();
    
    for (auto btn : _skills_buttons) {
        lv_obj_del(btn);
    }
    _skills_buttons.clear();
    
    // Create match auto buttons
    int x = 10, y = 10;
    int btn_width = 140;
    int btn_height = 40;
    int spacing = 10;
    
    for (size_t i = 0; i < _match_autos.size(); i++) {
        lv_obj_t* btn = lv_btn_create(_match_tab);
        lv_obj_set_pos(btn, x, y);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_user_data(btn, reinterpret_cast<void*>(i));
        lv_obj_add_event_cb(btn, match_btn_event_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(i));
        
        // Create label inside button
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, _match_autos[i].name.c_str());
        lv_obj_center(label);
        
        // Highlight if selected
        if (static_cast<int>(i) == _selected_match) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x00AA00), LV_PART_MAIN);
        }
        
        _match_buttons.push_back(btn);
        
        // Update position for next button
        x += btn_width + spacing;
        if (x + btn_width > 470) {
            x = 10;
            y += btn_height + spacing;
        }
    }
    
    // Create skills auto buttons
    x = 10;
    y = 10;
    
    for (size_t i = 0; i < _skills_autos.size(); i++) {
        lv_obj_t* btn = lv_btn_create(_skills_tab);
        lv_obj_set_pos(btn, x, y);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_user_data(btn, reinterpret_cast<void*>(i));
        lv_obj_add_event_cb(btn, skills_btn_event_cb, LV_EVENT_CLICKED, reinterpret_cast<void*>(i));
        
        // Create label inside button
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, _skills_autos[i].name.c_str());
        lv_obj_center(label);
        
        // Highlight if selected
        if (static_cast<int>(i) == _selected_skills) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x00AA00), LV_PART_MAIN);
        }
        
        _skills_buttons.push_back(btn);
        
        // Update position for next button
        x += btn_width + spacing;
        if (x + btn_width > 470) {
            x = 10;
            y += btn_height + spacing;
        }
    }
    
    // Send autonomous list to UI
    std::vector<std::string> match_names, skills_names;
    for (const auto& a : _match_autos) match_names.push_back(a.name);
    for (const auto& a : _skills_autos) skills_names.push_back(a.name);
    host::IPCClient::instance().send_auton_list(match_names, skills_names);
}

void Selector::match_btn_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    int index = static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
    Selector& sel = Selector::instance();
    
    // Update selection
    sel._selected_match = index;
    
    // Update description label
    if (index >= 0 && index < static_cast<int>(sel._match_autos.size())) {
        lv_label_set_text(sel._match_label, sel._match_autos[index].description.c_str());
        std::cout << "Selected match auto: " << sel._match_autos[index].name << std::endl;
    }
    
    // Update button styles
    sel.update_buttons();
}

void Selector::skills_btn_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    int index = static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
    Selector& sel = Selector::instance();
    
    // Update selection
    sel._selected_skills = index;
    
    // Update description label
    if (index >= 0 && index < static_cast<int>(sel._skills_autos.size())) {
        lv_label_set_text(sel._skills_label, sel._skills_autos[index].description.c_str());
        std::cout << "Selected skills auto: " << sel._skills_autos[index].name << std::endl;
    }
    
    // Update button styles
    sel.update_buttons();
}

void Selector::register_match_auto(const std::string& name,
                                    const std::string& description,
                                    std::function<void()> func) {
    _match_autos.emplace_back(name, description, func);
    
    // If already initialized, update UI
    if (_initialized) {
        update_buttons();
    }
}

void Selector::register_skills_auto(const std::string& name,
                                     const std::string& description,
                                     std::function<void()> func) {
    _skills_autos.emplace_back(name, description, func);
    
    // If already initialized, update UI
    if (_initialized) {
        update_buttons();
    }
}

int Selector::get_selected_match() {
    return _selected_match;
}

int Selector::get_selected_skills() {
    return _selected_skills;
}

void Selector::run_selected_match() {
    if (_selected_match >= 0 && _selected_match < static_cast<int>(_match_autos.size())) {
        std::cout << "Running match auto: " << _match_autos[_selected_match].name << std::endl;
        _match_autos[_selected_match].func();
    } else {
        std::cout << "No match auto selected!" << std::endl;
    }
}

void Selector::run_selected_skills() {
    if (_selected_skills >= 0 && _selected_skills < static_cast<int>(_skills_autos.size())) {
        std::cout << "Running skills auto: " << _skills_autos[_selected_skills].name << std::endl;
        _skills_autos[_selected_skills].func();
    } else {
        std::cout << "No skills auto selected!" << std::endl;
    }
}

void Selector::run_selected() {
    if (is_skills_mode()) {
        run_selected_skills();
    } else {
        run_selected_match();
    }
}

const std::vector<AutonRoutine>& Selector::get_match_autos() {
    return _match_autos;
}

const std::vector<AutonRoutine>& Selector::get_skills_autos() {
    return _skills_autos;
}

bool Selector::is_skills_mode() {
    if (!_tabview) return false;
    return lv_tabview_get_tab_act(_tabview) == 1;
}

// Auto-registration helper
AutoRegistrar::AutoRegistrar(const std::string& name,
                              const std::string& description,
                              std::function<void()> func,
                              bool is_skills) {
    if (is_skills) {
        Selector::instance().register_skills_auto(name, description, func);
    } else {
        Selector::instance().register_match_auto(name, description, func);
    }
}

} // namespace auton
