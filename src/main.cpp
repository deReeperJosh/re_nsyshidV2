#include "utils/logger.h"
#include <coreinit/filesystem.h>
#include <wups.h>
#include <wups/button_combo/api.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemButtonCombo.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>

#include <forward_list>

#include <malloc.h>

/**
    Mandatory plugin information.
    If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("re_nsyshid");
WUPS_PLUGIN_DESCRIPTION("This is just an example plugin written in C++");
WUPS_PLUGIN_VERSION("v0.1");
WUPS_PLUGIN_AUTHOR("deReeperJosh");
WUPS_PLUGIN_LICENSE("GPLv2");

#define LOG_FS_OPEN_CONFIG_ID             "logFSOpen"
#define BUTTON_COMBO_PRESS_DOWN_CONFIG_ID "pressDownItem"
#define BUTTON_COMBO_HOLD_CONFIG_ID       "holdItem"
#define OTHER_EXAMPLE_BOOL_CONFIG_ID      "otherBoolItem"
#define OTHER_EXAMPLE2_BOOL_CONFIG_ID     "other2BoolItem"
#define INTEGER_RANGE_EXAMPLE_CONFIG_ID   "intRangeExample"
#define MULTIPLE_VALUES_EXAMPLE_CONFIG_ID "multValueExample"

/**
    All of this defines can be used in ANY file.
    It's possible to split it up into multiple files.

**/

WUPS_USE_WUT_DEVOPTAB();        // Use the wut devoptabs
WUPS_USE_STORAGE("re_nsyshid"); // Unique id for the storage api

enum ExampleOptions {
    EXAMPLE_OPTION_1 = 0,
    EXAMPLE_OPTION_2 = 1,
    EXAMPLE_OPTION_3 = 2,
};

#define LOF_FS_OPEN_DEFAULT_VALUE     true
#define INTEGER_RANGE_DEFAULT_VALUE   10
#define MULTIPLE_VALUES_DEFAULT_VALUE EXAMPLE_OPTION_2

bool sLogFSOpen                    = LOF_FS_OPEN_DEFAULT_VALUE;
int sIntegerRangeValue             = INTEGER_RANGE_DEFAULT_VALUE;
ExampleOptions sExampleOptionValue = MULTIPLE_VALUES_DEFAULT_VALUE;

static std::forward_list<WUPSButtonComboAPI::ButtonCombo> sButtonComboInstances;

static WUPSButtonCombo_ComboHandle sPressDownExampleHandle(nullptr);
static WUPSButtonCombo_ComboHandle sHoldExampleHandle(nullptr);

WUPSButtonCombo_Buttons DEFAULT_PRESS_DOWN_BUTTON_COMBO = WUPS_BUTTON_COMBO_BUTTON_L | WUPS_BUTTON_COMBO_BUTTON_R;
WUPSButtonCombo_Buttons DEFAULT_PRESS_HOLD_COMBO        = WUPS_BUTTON_COMBO_BUTTON_L | WUPS_BUTTON_COMBO_BUTTON_R | WUPS_BUTTON_COMBO_BUTTON_DOWN;

/**
 * Callback that will be called if the config has been changed
 */
void boolItemChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in boolItemChanged: %d", newValue);
    if (std::string_view(LOG_FS_OPEN_CONFIG_ID) == item->identifier) {
        sLogFSOpen = newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, newValue);
    } else if (std::string_view(OTHER_EXAMPLE_BOOL_CONFIG_ID) == item->identifier) {
        DEBUG_FUNCTION_LINE_ERR("Other bool value has changed to %d", newValue);
    } else if (std::string_view(OTHER_EXAMPLE2_BOOL_CONFIG_ID) == item->identifier) {
        DEBUG_FUNCTION_LINE_ERR("Other2 bool value has changed to %d", newValue);
    }
}

void integerRangeItemChanged(ConfigItemIntegerRange *item, int newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in integerRangeItemChanged: %d", newValue);
    // If the value has changed, we store it in the storage.
    if (std::string_view(LOG_FS_OPEN_CONFIG_ID) == item->identifier) {
        sIntegerRangeValue = newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, newValue);
    }
}

void multipleValueItemChanged(ConfigItemIntegerRange *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in multipleValueItemChanged: %d", newValue);
    // If the value has changed, we store it in the storage.
    if (std::string_view(MULTIPLE_VALUES_EXAMPLE_CONFIG_ID) == item->identifier) {
        sExampleOptionValue = (ExampleOptions) newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, sExampleOptionValue);
    }
}

void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in buttonComboItemChanged: %d for %s", newValue, item->identifier);
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    // To use the C++ API, we create new WUPSConfigCategory from the root handle!
    WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

    // The functions of the Config API come in two variants: One that throws an exception, and another one which doesn't
    // To use the Config API without exception see the example below this try/catch block.
    try {
        // To select value from an enum WUPSConfigItemMultipleValues fits the best.
        constexpr WUPSConfigItemMultipleValues::ValuePair possibleValues[] = {
                {EXAMPLE_OPTION_1, "Option 1"},
                {EXAMPLE_OPTION_2, "Option 2"},
                {EXAMPLE_OPTION_3, "Option 3"},
        };

        // It comes in two variants.
        // - "WUPSConfigItemMultipleValues::CreateFromValue" will take a default and current **value**
        // - "WUPSConfigItemMultipleValues::CreateFromIndex" will take a default and current **index**
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(MULTIPLE_VALUES_EXAMPLE_CONFIG_ID, "Select an option!",
                                                               MULTIPLE_VALUES_DEFAULT_VALUE, sExampleOptionValue,
                                                               possibleValues,
                                                               nullptr));
    } catch (std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Creating config menu failed: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
    WUPSStorageAPI::SaveStorage();
}

/**
    Gets called ONCE when the plugin was loaded.
**/
INITIALIZE_PLUGIN() {
    // Logging only works when compiled with `make DEBUG=1`. See the README for more information.
    initLogging();
    DEBUG_FUNCTION_LINE("INITIALIZE_PLUGIN of re_nsyshid!");

    WUPSConfigAPIOptionsV1 configOptions = {.name = "re_nsyshid"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }

    deinitLogging();
}

/**
    Gets called when the plugin will be unloaded.
**/
DEINITIALIZE_PLUGIN() {
    // Remove all button combos from this plugin.
    sButtonComboInstances.clear();
    DEBUG_FUNCTION_LINE("DEINITIALIZE_PLUGIN of re_nsyshid!");
}

/**
    Gets called when an application starts.
**/
ON_APPLICATION_START() {
    initLogging();

    DEBUG_FUNCTION_LINE("ON_APPLICATION_START of re_nsyshid!");
}

/**
 * Gets called when an application actually ends
 */
ON_APPLICATION_ENDS() {
    deinitLogging();
}

/**
    Gets called when an application request to exit.
**/
ON_APPLICATION_REQUESTS_EXIT() {
    DEBUG_FUNCTION_LINE_INFO("ON_APPLICATION_REQUESTS_EXIT of re_nsyshid!");
}
