/**
 * @file selector.hpp
 * @brief Autonomous Selector with LVGL UI
 * 
 * This header provides the autonomous selector interface that uses
 * LVGL tabview for selecting autonomous routines.
 */

#ifndef AUTON_SELECTOR_HPP
#define AUTON_SELECTOR_HPP

#include "liblvgl/lvgl.h"
#include <string>
#include <vector>
#include <functional>

namespace auton {

/**
 * Autonomous routine information
 */
struct AutonRoutine {
    std::string name;
    std::string description;
    std::function<void()> func;
    
    AutonRoutine(const std::string& n, const std::string& d, std::function<void()> f)
        : name(n), description(d), func(f) {}
};

/**
 * Autonomous Selector class
 * 
 * Provides a tabbed LVGL interface for selecting between match
 * and skills autonomous routines.
 */
class Selector {
public:
    /**
     * Gets the singleton instance.
     *
     * @return Reference to the Selector instance
     */
    static Selector& instance();

    // Delete copy/move constructors
    Selector(const Selector&) = delete;
    Selector& operator=(const Selector&) = delete;
    Selector(Selector&&) = delete;
    Selector& operator=(Selector&&) = delete;

    /**
     * Initializes the selector UI.
     * Creates the LVGL tabview and button matrix.
     */
    void init();

    /**
     * Destroys the selector UI.
     */
    void destroy();

    /**
     * Checks if the selector is initialized.
     *
     * @return True if initialized
     */
    bool is_initialized();

    /**
     * Registers a match autonomous routine.
     *
     * @param name The name displayed on the button
     * @param description A description of the routine
     * @param func The function to execute
     */
    void register_match_auto(const std::string& name, 
                             const std::string& description,
                             std::function<void()> func);

    /**
     * Registers a skills autonomous routine.
     *
     * @param name The name displayed on the button
     * @param description A description of the routine
     * @param func The function to execute
     */
    void register_skills_auto(const std::string& name,
                              const std::string& description,
                              std::function<void()> func);

    /**
     * Gets the currently selected match autonomous.
     *
     * @return Index of the selected match auto, or -1 if none
     */
    int get_selected_match();

    /**
     * Gets the currently selected skills autonomous.
     *
     * @return Index of the selected skills auto, or -1 if none
     */
    int get_selected_skills();

    /**
     * Runs the selected match autonomous.
     */
    void run_selected_match();

    /**
     * Runs the selected skills autonomous.
     */
    void run_selected_skills();

    /**
     * Runs the appropriate autonomous based on current tab.
     */
    void run_selected();

    /**
     * Gets the match autonomous list.
     *
     * @return Reference to the match auto vector
     */
    const std::vector<AutonRoutine>& get_match_autos();

    /**
     * Gets the skills autonomous list.
     *
     * @return Reference to the skills auto vector
     */
    const std::vector<AutonRoutine>& get_skills_autos();

    /**
     * Gets whether skills tab is active.
     *
     * @return True if skills tab is selected
     */
    bool is_skills_mode();

private:
    Selector();
    ~Selector();

    void create_ui();
    void update_buttons();

    static void match_btn_event_cb(lv_event_t* e);
    static void skills_btn_event_cb(lv_event_t* e);

    bool _initialized;
    
    // LVGL objects
    lv_obj_t* _tabview;
    lv_obj_t* _match_tab;
    lv_obj_t* _skills_tab;
    lv_obj_t* _match_label;
    lv_obj_t* _skills_label;
    
    // Button containers
    std::vector<lv_obj_t*> _match_buttons;
    std::vector<lv_obj_t*> _skills_buttons;
    
    // Autonomous routines
    std::vector<AutonRoutine> _match_autos;
    std::vector<AutonRoutine> _skills_autos;
    
    // Selection state
    int _selected_match;
    int _selected_skills;
};

/**
 * Auto-registration helper class
 */
class AutoRegistrar {
public:
    AutoRegistrar(const std::string& name, 
                  const std::string& description,
                  std::function<void()> func,
                  bool is_skills);
};

} // namespace auton

/**
 * Macro for registering match autonomous routines
 */
#define REGISTER_MATCH_AUTO(name, desc, func) \
    static auton::AutoRegistrar _match_reg_##func(name, desc, func, false)

/**
 * Macro for registering skills autonomous routines
 */
#define REGISTER_SKILLS_AUTO(name, desc, func) \
    static auton::AutoRegistrar _skills_reg_##func(name, desc, func, true)

#endif // AUTON_SELECTOR_HPP
