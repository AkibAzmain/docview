/*
    Copyright (C) 2020 Akib Azmain
    
    This file is a part of Docview.
    
    Docview is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Docview is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Docview. If not, see <http://www.gnu.org/licenses/>.
*/

#include <docview.hpp>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/widget.h>
#include <gtkmm/modelbutton.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/treestore.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtkmm/label.h>
#include <gtkmm/stack.h>
#include <gtkmm/stackswitcher.h>
#include <gtkmm/switch.h>
#include <gtkmm/revealer.h>
#include <gtkmm/expander.h>
#include <gtkmm/searchbar.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/dialog.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/spinbutton.h>
#include <webkit2/webkit2.h>
#include <glibmm/ustring.h>
#include <pango/pango-font.h>
#include <libxml++/parsers/domparser.h>
#include <libxml++/document.h>
#include <libxml++/nodes/node.h>
#include <libxml++/nodes/element.h>
#include <memory>
#include <exception>
#include <iostream>
#include <functional>
#include <array>
#include <utility>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>

// This global variable will contain pointer to Gtk::Builder (managed by Glib::RefPtr)
Gtk::Builder* builder = nullptr;

/**
 * @brief Returns pointer to widget specified by given id
 * 
 * @tparam widget_type type of widget
 * @param id id of widget
 * @return pointer to widget
 */
template <class widget_type = Gtk::Widget>
widget_type* get_widget(std::string id)
{
    widget_type* widget_ptr;
    builder->get_widget(id, widget_ptr);
    return widget_ptr;
}

class configuration
{
private:

    // The parser object
    xmlpp::DomParser parser;

    // Pointer to document object
    xmlpp::Document* document;

    // The name of configuration file
    std::string config_file;

    /**
     * @brief Returns pointer to node at given path
     * 
     * @param root search root
     * @param path path of path
     * @param create whether to create new node if does exist
     * @return pointer to node at path
     */
    xmlpp::Node* get_node(xmlpp::Node* root, std::vector<std::string> path, bool create = false)
    {
        if (path.size() == 0)
            return root;
        
        for (auto& child : root->get_children())
        {
            if (child->get_name() == path[0])
                return get_node(child, std::vector<std::string>(path.begin() + 1, path.end()), create);
        }

        if (create)
        {
            auto element = root->add_child(path[0]);
            return get_node(element, std::vector<std::string>(path.begin() + 1, path.end()), create);
        }

        return nullptr;
    }

public:

    /**
     * @brief Constructs a new configuration object
     * 
     */
    configuration()
        : parser(),
        document(nullptr),

        // Setup config file path according to platform
        config_file(
            #ifdef __linux__
                std::filesystem::path("/home")
            #else
                std::filesystem::path("C:/Users")
            #endif
            / std::getenv("USER") /
            #ifdef __linux__
                std::filesystem::path(".local/share/Docview")
            #endif
            / "docview.xml"
        )
    {

        // Make sure directory exists
        if (!std::filesystem::exists(config_file))
        {
            std::filesystem::create_directories(config_file);
        }

        // Try to parse config file
        try
        {
            parser.parse_file(config_file);
            document = parser.get_document();
            if (document->get_root_node()->get_name() != "docview") throw xmlpp::exception("");
        }

        // On failure, rewrite config file
        catch (xmlpp::exception&)
        {

            // Clear config file content
            std::ofstream(config_file).flush();
            document = new xmlpp::Document;
            document->create_root_node("docview");
        }
    }

    /**
     * @brief Saves configuration and destroy the configuration object
     * 
     */
    ~configuration()
    {
        document->write_to_file(config_file);
    }

    /**
     * @brief Sets the value of a node
     * 
     * @param path path of node
     * @param value the value to set
     */
    void set_value(std::vector<std::string> path, Glib::ustring value)
    {

        // Get the node at given path, create if doesn't exist
        auto element = dynamic_cast<xmlpp::Element*>(get_node(document->get_root_node(), path, true));

        // Set the value
        element->set_child_text(value);
    }

    /**
     * @brief Returns the value of a node
     * 
     * @param path path of node
     * @return the value of node
     */
    Glib::ustring get_value(std::vector<std::string> path)
    {

        // Get the node at given path
        auto element = dynamic_cast<xmlpp::Element*>(get_node(document->get_root_node(), path));

        // Make sure element isn't nullptr, return empty string on failure
        if (!element) return Glib::ustring();

        // Make sure element has a text node isn't nullptr, return empty string on failure
        if (!element->get_child_text()) return Glib::ustring();

        // Return the value
        return element->get_child_text()->get_content();
    }
};

int main(int argc, char** argv)
{

    // Create the configuration object
    configuration config;

    // Create new Gtk::Application object
	auto app = Gtk::Application::create(argc, argv, "org.docview");

    // Create new Gtk::Builder object
    auto builder = Gtk::Builder::create();

    // Set the global builder variable to enable the use of function get_widget
    ::builder = builder.get();

    // Register WebKit's WebView and Settings widget to prevent builder errors
    webkit_web_view_new();
    webkit_settings_new();

    // Builder the UI from .ui file generated by glade
    try
    {
        builder->add_from_file(std::filesystem::path(ASSETS_DIR) / "window.ui");
    }

    // On any exception, print a error message and exit
    catch (Glib::Error& exception)
    {
        std::cerr << "Exception occurred: what(): " << exception.what() << std::endl;
        std::terminate();
    }

    // Get pointers to widgets from builder
    auto window = get_widget<Gtk::Window>("window_main");
    auto contents = get_widget<Gtk::Paned>("contents");
    auto sidebar_toggle_button = get_widget<Gtk::ToggleButton>("sidebar_toggle_button");
    auto about_button = get_widget<Gtk::ModelButton>("about_button");
    auto about_dialog = get_widget<Gtk::AboutDialog>("about_dialog");
    auto preferences_button = get_widget<Gtk::ModelButton>("preferences_button");
    auto preferences_dialog = get_widget<Gtk::Dialog>("preferences_dialog");
    auto quit_button = get_widget<Gtk::Button>("quit_button");
    auto sidebar_tree = get_widget<Gtk::TreeView>("sidebar_tree");
    auto tab_switcher = get_widget<Gtk::StackSwitcher>("tabs");
    auto stack = get_widget<Gtk::Stack>("doc_webviews");
    auto webview_refresh_button = get_widget<Gtk::Button>("webview_refresh_button");
    auto webview_find_button = get_widget<Gtk::ModelButton>("webview_find_button");
    auto webview_find_bar = get_widget<Gtk::SearchBar>("webview_find_bar");
    auto webview_find = get_widget<Gtk::SearchEntry>("webview_find");
    auto webview_find_previous_button = get_widget<Gtk::Button>("webview_find_previous");
    auto webview_find_next_button = get_widget<Gtk::Button>("webview_find_next");
    auto history_previous_button = get_widget<Gtk::Button>("history_previous");
    auto history_next_button = get_widget<Gtk::Button>("history_next");
    auto new_tab_button = get_widget<Gtk::Button>("new_tab_button");
    auto close_tab_button = get_widget<Gtk::ModelButton>("close_tab_button");
    auto search_entry = get_widget<Gtk::SearchEntry>("search_entry");
    auto title = get_widget<Gtk::Stack>("title");
    auto title_label = get_widget<Gtk::Label>("title_label");
    auto webview_settings = gtk_builder_get_object(builder->gobj(), "webview_settings");
    auto preferences_documentation_search_path =
        get_widget<Gtk::TextView>("preferences_documentation_search_path");
    auto preferences_max_search_results = get_widget<Gtk::SpinButton>("preferences_max_search_results");
    auto preferences_use_system_fonts = get_widget<Gtk::Switch>("preferences_use_system_fonts");
    auto preferences_fonts = get_widget<Gtk::Revealer>("preferences_fonts");
    auto preferences_default_font = get_widget<Gtk::FontButton>("preferences_default_font");
    auto preferences_monospace_font = get_widget<Gtk::FontButton>("preferences_monospace_font");
    auto preferences_extension_search_path_revealer =
        get_widget<Gtk::Revealer>("preferences_extension_search_path_revealer");
    auto preferences_extension_search_path_expander =
        get_widget<Gtk::Expander>("preferences_extension_search_path_expander");
    auto preferences_extension_list = get_widget<Gtk::TreeView>("preferences_extension_list");
    auto preferences_extension_search_path =
        get_widget<Gtk::TextView>("preferences_extension_search_path");
    auto preferences_close_button = get_widget<Gtk::Button>("preferences_close_button");
    Glib::RefPtr<Gtk::TextBuffer> preferences_extension_search_path_buffer = Gtk::TextBuffer::create();
    Glib::RefPtr<Gtk::TextBuffer> preferences_documentation_search_path_buffer = Gtk::TextBuffer::create();

    // This structure contains the contents of sidebar
    Glib::RefPtr<Gtk::TreeStore> sidebar_contents;

    // This structure contains search results of sidebar
    Glib::RefPtr<Gtk::ListStore> sidebar_search_results;

    // Column of sidebar tree holding title
    Gtk::TreeModelColumn<Glib::ustring> sidebar_column_title;

    // Column of sidebar tree holding pointer to node
    Gtk::TreeModelColumn<const docview::doc_tree_node*> sidebar_column_node;

    // This array holds all tabs
    std::vector<Gtk::Widget*> tabs;

    // This variable holds pointer to find controller of currently visible webview
    WebKitFindController* webview_finder = nullptr;

    // Vector holding all root nodes provided by libdocview
    std::vector<std::pair<const docview::doc_tree_node*, std::filesystem::path>> document_root_nodes;

    // Vector holding all loaded extension
    std::vector<std::filesystem::path> loaded_extensions;

    // List of all known extensions in an showable format
    Glib::RefPtr<Gtk::ListStore> extension_list_contents;

    // Column of extension list holding name
    Gtk::TreeModelColumn<Glib::ustring> extension_list_column_name;

    // Column of extension list holding is the extension enabled
    Gtk::TreeModelColumn<bool> extension_list_column_enabled;

    // Column of extension list holding path
    Gtk::TreeModelColumn<std::string> extension_list_column_path;

    // Declare all lambda function
    std::function<void()> on_sidebar_toggle_button_clicked;
    std::function<void()> on_sidebar_resized;
    std::function<void()> on_about_button_clicked;
    std::function<void()> on_preferences_button_clicked;
    std::function<void(const Gtk::TreeModel::Path&, Gtk::TreeView::Column*)> on_sidebar_option_selected;
    void(*on_webview_load_change)(WebKitWebView*, WebKitLoadEvent, void*);
    std::function<void()> on_title_changed;
    std::function<void()> on_active_tab_changed;
    std::function<void()> on_tab_added;
    std::function<void()> on_tab_closed;
    std::function<void()> on_webview_refresh_button_clicked;
    std::function<void()> on_webview_find_button_clicked;
    std::function<void()> on_webview_find_bar_state_changed;
    std::function<void()> on_webview_find_text_changed;
    std::function<void()> on_webview_find_previous;
    std::function<void()> on_webview_find_next;
    std::function<void()> on_history_previous;
    std::function<void()> on_history_next;
    std::function<void()> on_search_changed;
    std::function<void()> on_quit_button_clicked;
    std::function<void()> on_preferences_documentation_search_path_unfocused;
    std::function<void()> on_preferences_use_system_fonts_changed;
    std::function<void()> on_preferences_default_font_changed;
    std::function<void()> on_preferences_monospace_font_changed;
    std::function<void()> on_preferences_extension_search_path_expander_state_changed;
    std::function<void(const Gtk::TreeModel::Path&, Gtk::TreeView::Column*)>
        on_preferences_extension_enable_toggled;
    std::function<void()> on_preferences_extension_search_path_unfocused;
    std::function<void()> on_preferences_close_button_clicked;
    std::function<void(const docview::doc_tree_node*, Gtk::TreeStore::iterator)> build_tree;

    // Lambda function to call on sidebar toggle button clicked
    on_sidebar_toggle_button_clicked = [&]() -> void
    {

        // Used to store sidebar size
        static int sidebar_size;

        // If sidebar enabled, restore previous size
        if (sidebar_toggle_button->get_active())
        {
            contents->set_position(sidebar_size);
        }

        // If sidebar disabled, save sidebar size, set it's size to 0
        else
        {
            sidebar_size = contents->get_position();
            contents->set_position(0);
        }
    };

    // Lambda function to call on sidebar resized
    on_sidebar_resized = [&]() -> void
    {

        // If user tries resize sidebar to less than 200 pixels, set the size to 200 pixel
        if (contents->get_position() < 200 && sidebar_toggle_button->get_active())
        {
            contents->set_position(200);
        }

        // If user tries to change the size of sidebar when it's hidden, set the size to 0 pixel
        else if (!sidebar_toggle_button->get_active())
        {
            contents->set_position(0);
        }
    };

    // Lambda function to call on about button clicked
    on_about_button_clicked = [&]() -> void
    {

        // User wants to see the about dialog, present that
        about_dialog->present();
    };

    // Lambda function to call on preferences button clicked
    on_preferences_button_clicked = [&]() -> void
    {

        // User want the preferences dialog dialog, present that
        preferences_dialog->present();
    };

    // Lambda function to call on sidebar option selected
    on_sidebar_option_selected = [&](const Gtk::TreeModel::Path& path, Gtk::TreeView::Column*) -> void
    {

        // Iterator to the row
        Gtk::TreeModel::iterator it = sidebar_tree->get_model()->get_iter(path);

        // Make sure the iterator is valid
        if (path)
        {

            // Dereference the iterator
            Gtk::TreeModel::Row row = *it;

            webkit_web_view_load_uri(WEBKIT_WEB_VIEW(stack->get_visible_child()->gobj()),
                docview::get_doc(row[sidebar_column_node]).first.c_str()
            );
        }
    };

    // Lambda function to call on webview load change
    on_webview_load_change = [](WebKitWebView* webview, WebKitLoadEvent event, void*) -> void
    {

        // Title property of webview
        auto title = ((Gtk::Stack*)Glib::wrap(GTK_WIDGET(webview))->get_parent())->child_property_title(
            *Glib::wrap(GTK_WIDGET(webview))
        );

        // Title of webview, maybe nullptr
        const char* webview_title = webkit_web_view_get_title(webview);

        // If the page has finished loading, change the title of window to title of page
        if (event == WEBKIT_LOAD_FINISHED)
        {

            // Make sure webview title isn't nullptr or empty
            if (webview_title && Glib::ustring(webview_title) != Glib::ustring())
                title = webview_title;
            else
                title = "<No title>";
        }

        // Page is being loaded, show loading in title
        else
        {
            title = "Loading";
        }
    };

    // Lambda function to call on title changed
    on_title_changed = [&]() -> void
    {

        // Change the window title
        title_label->set_label(
            stack->child_property_title(*stack->get_visible_child()) + Glib::ustring(" - Docview")
        );
    };

    // Lambda function to call on active tab change
    on_active_tab_changed = [&]() -> void
    {

        // The active tab is changed, so title should be changed
        on_title_changed();

        // The active tab is changed, so setup the find bar if open
        on_webview_find_bar_state_changed();
    };

    // Lambda function to call on tab added
    on_tab_added = [&]() -> void
    {

        // Tab number, automatically initialized with zero
        static unsigned int tab_num;

        // Increment tab_num, as 0 is already used by initial tab
        tab_num++;

        // Create and configure new webview
        Gtk::Widget* webview = Glib::wrap(webkit_web_view_new());
        webkit_web_view_set_settings(
            WEBKIT_WEB_VIEW(webview->gobj()),
            WEBKIT_SETTINGS(webview_settings)
        );
        webkit_web_view_load_uri(
            WEBKIT_WEB_VIEW(webview->gobj()),
            (std::string("file://") + ASSETS_DIR + "/welcome.html").c_str()
        );
        g_signal_connect(webview->gobj(), "load-changed",
            G_CALLBACK(on_webview_load_change), nullptr);

        // Create new tab, present it to user
        stack->add(*webview, std::to_string(tab_num), "Empty Page");
        webview->show();
        stack->set_visible_child(*webview);

        // Show tab_switcher in title and show changes
        if (tabs.size() >= 1)
            title->set_visible_child(*tab_switcher);
        window->show_all_children();

        // Configure event handler
        stack->child_property_title(*webview).signal_changed().connect(sigc::mem_fun(
            on_title_changed, &std::function<void()>::operator()
        ));

        // Insert the tab to tabs
        tabs.push_back(webview);
    };

    // Lambda function to call on tab closed
    on_tab_closed = [&]() -> void
    {

        // If there will be no tabs left after removal, close the window
        if (tabs.size() - 1 == 0)
        {
            window->hide();
            return;
        }

        // The tab to close
        Gtk::Widget* tab_to_close = stack->get_visible_child();

        // Move all widgets to the new stack
        for (unsigned long i = 0; i < tabs.size(); i++)
        {

            // If the current tab is the tab to close, don't add it and remove it from tabs
            if (tabs[i] == tab_to_close)
            {
                stack->remove(*tab_to_close);
                tabs.erase(tabs.begin() + i--);
            }
        }

        // If only one tab is left, show only tab title in title
        if (tabs.size() == 1)
        {
            title->set_visible_child(*title_label);
        }

        // Update window
        window->show_all_children();
    };

    // Lambda function to call on webview refresh button clicked
    on_webview_refresh_button_clicked = [&]() -> void
    {

        // Reload webview
        webkit_web_view_reload(WEBKIT_WEB_VIEW(stack->get_visible_child()->gobj()));
    };

    // Lambda function to call on webview clicked button clicked
    on_webview_find_button_clicked = [&]() -> void
    {

        // Open find bar
        webview_find_bar->set_search_mode();
    };

    // Lambda function to call on webview find bar state changed
    on_webview_find_bar_state_changed = [&]() -> void
    {

        // The find bar is either opened or closed, so empty the find query
        webview_find->set_text(Glib::ustring());

        // If search mode in on, setup the webview find controller
        if (webview_find_bar->get_search_mode())
        {
            webview_finder = webkit_web_view_get_find_controller(
                WEBKIT_WEB_VIEW(stack->get_visible_child()->gobj())
            );
        }
    };

    // Lambda function to call on webview find query changed
    on_webview_find_text_changed = [&]() -> void
    {

        // Find query isn't empty, find
        if (webview_find->get_text() != Glib::ustring())
        {
            webkit_find_controller_search(
                webview_finder,
                webview_find->get_text().c_str(),
                WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE,
                G_MAXUINT
            );
        }

        // Otherwise, end the search
        else
        {
            webkit_find_controller_search_finish(webview_finder);
        }
    };

    // Lambda function to call on webview find previous button clicked
    on_webview_find_previous = [&]() -> void
    {

        // Go to previous result, only if possible
        if (webview_finder && webview_find->get_text() != Glib::ustring())
            webkit_find_controller_search_previous(webview_finder);
    };

    // Lambda function to call on webview find next button clicked
    on_webview_find_next = [&]() -> void
    {

        // Go to next result, only if possible
        if (webview_finder && webview_find->get_text() != Glib::ustring())
            webkit_find_controller_search_next(webview_finder);
    };

    // Lambda function to call on history previous button clicked
    on_history_previous = [&]() -> void
    {

        // Go to previous result, only if possible
        Gtk::Widget* webview = stack->get_visible_child();
        if (webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(webview->gobj())))
            webkit_web_view_go_back(WEBKIT_WEB_VIEW(webview->gobj()));
    };

    // Lambda function to call on history next button clicked
    on_history_next = [&]() -> void
    {

        // Go to next result, only if possible
        Gtk::Widget* webview = stack->get_visible_child();
        if (webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(webview->gobj())))
            webkit_web_view_go_forward(WEBKIT_WEB_VIEW(webview->gobj()));
    };

    // Lambda function to call on search query changed
    on_search_changed = [&]() -> void
    {

        // If search entry is empty, reset sidebar to it's original state
        if (search_entry->get_text().empty())
        {
            sidebar_tree->set_model(sidebar_contents);
            window->show_all_children();
            return;
        }

        // Change the data holder of sidebar
        sidebar_tree->set_model(sidebar_search_results);

        // Search through all created document nodes till now
        // TODO: Use of deprecated function
        std::vector<const docview::doc_tree_node*> matches =
            docview::search(search_entry->get_text(), document_root_nodes);

        // Clear the sidebar
        sidebar_search_results->clear();

        // Show the search results in sidebar
        for (
            unsigned long i = 0;
            i < matches.size() && int(i) < preferences_max_search_results->get_value_as_int();
            i++
        )
        {

            // Create a new row
            auto row = sidebar_search_results->append();

            // Set value of columns
            (*row)[sidebar_column_title] = matches[i]->title;
            (*row)[sidebar_column_node] = matches[i];
        }

        window->show_all_children();
    };

    // Lambda function to call on quit button clicked
    on_quit_button_clicked = [&]() -> void
    {
        window->hide();
    };

    // Lambda function to call on extension search path textview loses focus
    on_preferences_documentation_search_path_unfocused = [&]() -> void
    {

        // If the textview has got focus just now, do nothing
        if (preferences_documentation_search_path->property_has_focus() == true) return;

        // Set the config value in configuration file
        config.set_value(
            {"preferences", "documentations", "search_path"},
            preferences_documentation_search_path_buffer->get_text()
        );

        // Break the input string into paths
        std::vector<std::filesystem::path> paths;
        {
            std::string path;
            std::stringstream stream(preferences_documentation_search_path_buffer->get_text());
            while (std::getline(stream, path))
                paths.push_back(path);
        }

        // Clear the sidebar
        sidebar_contents->clear();

        // Clear the all known nodes
        document_root_nodes.clear();

        // Populate the sidebar again
        for (auto& path : paths)
        {

            // Make sure path is an directory
            if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
                continue;

            // For every file, try to parse it, do nothing on failure
            for (auto& file : std::filesystem::directory_iterator(path))
            {
                try
                {
                    const docview::doc_tree_node* node = docview::get_doc_tree(file);
                    if (node)
                    {
                        document_root_nodes.push_back(std::make_pair(node, file.path()));
                        build_tree(node, sidebar_contents->append());
                    }
                }
                catch (...) {}
            }
        }

        // Change sidebar contents
        sidebar_tree->set_model(sidebar_contents);
        search_entry->set_text(Glib::ustring());

        window->show_all_children();
    };

    // Lambda function to call on preferences use system fonts switch changed
    on_preferences_use_system_fonts_changed = [&]() -> void
    {

        // Update configuration
        config.set_value(
            {"preferences", "interface", "fonts", "use_system"},
            std::to_string(preferences_use_system_fonts->get_active())
        );

        // Show or hide other options according to state
        preferences_fonts->set_reveal_child(!preferences_use_system_fonts->get_active());

        // If enabled, use system fonts
        if (preferences_use_system_fonts->get_active())
        {

            // TODO: These are just dummy fonts, get actual ones from system
            webkit_settings_set_default_font_family(WEBKIT_SETTINGS(webview_settings), "Ubuntu");
            webkit_settings_set_default_font_size(WEBKIT_SETTINGS(webview_settings), 14);
            webkit_settings_set_monospace_font_family(WEBKIT_SETTINGS(webview_settings), "Ubuntu Mono");
            webkit_settings_set_default_monospace_font_size(WEBKIT_SETTINGS(webview_settings), 14);
        }

        // Otherwise of user selected fonts
        else
        {
            on_preferences_default_font_changed();
            on_preferences_monospace_font_changed();
        }

        window->show_all_children();
    };

    // Lambda function to call on default font preferences changed
    on_preferences_default_font_changed = [&]() -> void
    {

        // TODO: Use of deprecated function, replace with an alternative
        Glib::ustring font_id = preferences_default_font->get_font_name();
        Glib::ustring font_family;
        int font_size;
        {

            // Parse font ID to get size and family
            auto font = pango_font_description_from_string(font_id.c_str());
            font_family = pango_font_description_get_family(font);
            font_size = pango_font_description_get_size(font) / PANGO_SCALE;
            pango_font_description_free(font);
        }

        // Set fonts
        webkit_settings_set_default_font_family(WEBKIT_SETTINGS(webview_settings), font_family.c_str());
        webkit_settings_set_default_font_size(WEBKIT_SETTINGS(webview_settings), font_size);

        // Update configuration
        config.set_value({"preferences", "interface", "fonts", "default"}, font_id);

        window->show_all_children();
    };

    // Lambda function to call on monospace font preferences changed
    on_preferences_monospace_font_changed = [&]() -> void
    {

        // TODO: Use of deprecated function, replace with an alternative
        Glib::ustring font_id = preferences_monospace_font->get_font_name();
        Glib::ustring font_family;
        int font_size;
        {

            // Parse font ID to get size and family
            auto font = pango_font_description_from_string(font_id.c_str());
            font_family = pango_font_description_get_family(font);
            font_size = pango_font_description_get_size(font) / PANGO_SCALE;
            pango_font_description_free(font);
        }

        // Set fonts
        webkit_settings_set_monospace_font_family(WEBKIT_SETTINGS(webview_settings), font_family.c_str());
        webkit_settings_set_default_monospace_font_size(WEBKIT_SETTINGS(webview_settings), font_size);

        // Update configuration
        config.set_value({"preferences", "interface", "fonts", "monospace"}, font_id);

        window->show_all_children();
    };

    // Lambda function to call on sidebar option selected
    on_preferences_extension_enable_toggled =
        [&](const Gtk::TreeModel::Path& path, Gtk::TreeView::Column*) -> void
    {

        // Iterator to the row
        Gtk::TreeModel::iterator it = extension_list_contents->get_iter(path);

        // Make sure the iterator is valid
        if (path)
        {

            // Dereference the iterator
            Gtk::TreeModel::Row row = *it;

            config.set_value(
                {
                    "preferences", "extensions", "list",
                    (std::string)(Glib::ustring)row[extension_list_column_name], "enabled"
                },
                std::to_string(row[extension_list_column_enabled])
            );

            if (row[extension_list_column_enabled])
            {
                docview::load_ext((std::string)row[extension_list_column_path]);
                loaded_extensions.push_back((std::string)row[extension_list_column_path]);
            }
            else
            {
                docview::unload_ext((std::string)row[extension_list_column_path]);
                for (auto it = loaded_extensions.begin(); it != loaded_extensions.end(); it++)
                {
                    if (*it == (std::string)row[extension_list_column_path])
                    {
                        loaded_extensions.erase(it);
                        break;
                    }
                }
            }
        }

        // Update the sidebar
        on_preferences_documentation_search_path_unfocused();
    };

    // Lambda function to call on extension search path expander state changed
    on_preferences_extension_search_path_expander_state_changed = [&]() -> void
    {
        preferences_extension_search_path_revealer->set_reveal_child(
            preferences_extension_search_path_expander->get_expanded()
        );
    };

    // Lambda function to call on extension search path textview loses focus
    on_preferences_extension_search_path_unfocused = [&]() -> void
    {

        // If the textview has got focus just now, do nothing
        if (preferences_extension_search_path->property_has_focus() == true) return;

        // Set the config value in configuration file
        config.set_value(
            {"preferences", "extensions", "search_path"},
            preferences_extension_search_path_buffer->get_text()
        );

        // Break the input string into paths
        std::vector<std::filesystem::path> paths;
        {
            std::string path;
            std::stringstream stream(preferences_extension_search_path_buffer->get_text());
            while (std::getline(stream, path))
                paths.push_back(path);
        }

        // Clear the list
        extension_list_contents->clear();

        // Unload all extensions
        for (auto& extension : loaded_extensions)
        {
            docview::unload_ext(extension);
        }
        loaded_extensions.clear();

        // Create the list again
        for (auto& path : paths)
        {
            if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
                continue;

            for (auto& file : std::filesystem::directory_iterator(path))
                if (std::filesystem::is_regular_file(file.path()))
                {

                    // Create a new row
                    auto row = extension_list_contents->append();

                    // Set value of columns
                    (*row)[extension_list_column_name] = Glib::ustring(std::string(file.path().filename()));
                    (*row)[extension_list_column_enabled] =
                        (config.get_value(
                            {"preferences", "extensions", "list",
                            std::string(file.path().filename()), "enabled"}
                        )  == "1") ? true : false;
                    (*row)[extension_list_column_path] = std::string(std::filesystem::absolute(file.path()));

                    if ((*row)[extension_list_column_enabled])
                    {
                        try
                        {
                            docview::load_ext(std::filesystem::absolute(file.path()));
                            loaded_extensions.push_back(std::filesystem::absolute(file.path()));
                        }
                        catch (std::runtime_error&)
                        {
                            (*row)[extension_list_column_enabled] = 0;
                            config.set_value(
                                {
                                    "preferences", "extensions", "list",
                                    std::string(file.path().filename()), "enabled"
                                },
                                "0"
                            );
                        }
                    }
                }
        }

        // Update the sidebar
        on_preferences_documentation_search_path_unfocused();

        window->show_all_children();
    };

    // Lambda function to call on preferences close button clicked
    on_preferences_close_button_clicked = [&]() -> void
    {

        // Close the preferences dialog
        preferences_dialog->hide();
    };

    // Set icons
    window->set_icon_from_file(std::string(ICONS48_DIR) + "/docview48x48.png");
    about_dialog->property_logo() =
        Gdk::Pixbuf::create_from_file(std::string(ICONS128_DIR) + "/docview128x128.png");

    // Load configuration
    preferences_documentation_search_path_buffer->set_text(config.get_value(
        {"preferences", "documentations", "search_path"}
    ));
    preferences_use_system_fonts->set_active(config.get_value(
        {"preferences", "interface", "fonts", "use_system"}
    ) == "0" ? false : true);
    preferences_default_font->set_font_name(config.get_value(
        {"preferences", "interface", "fonts", "default"}
    ) == "" ? "Ubuntu 14" : config.get_value(
        {"preferences", "interface", "fonts", "default"}
    ));
    preferences_monospace_font->set_font_name(config.get_value(
        {"preferences", "interface", "fonts", "monospace"}
    ) == "" ? "Ubuntu Mono 14" : config.get_value(
        {"preferences", "interface", "fonts", "monospace"}
    ));
    preferences_extension_search_path_buffer->set_text(config.get_value(
        {"preferences", "extensions", "search_path"}
    ));
    preferences_max_search_results->set_value(config.get_value(
        {"preferences", "interface", "search", "max_results"}
    ) == "" ? 500 : std::stoi(config.get_value(
        {"preferences", "interface", "search", "max_results"}
    )));

    // Configure widgets to call the above handlers in appropiate events
    sidebar_toggle_button->signal_clicked().connect(
        sigc::mem_fun(on_sidebar_toggle_button_clicked, &std::function<void()>::operator())
    );
    about_button->signal_clicked().connect(
        sigc::mem_fun(on_about_button_clicked, &std::function<void()>::operator())
    );
    preferences_button->signal_clicked().connect(
        sigc::mem_fun(on_preferences_button_clicked, &std::function<void()>::operator())
    );
    contents->property_position().signal_changed().connect(
        sigc::mem_fun(on_sidebar_resized, &std::function<void()>::operator())
    );
    sidebar_tree->signal_row_activated().connect(sigc::mem_fun(on_sidebar_option_selected,
        &std::function<void(const Gtk::TreeModel::Path&, Gtk::TreeView::Column*)>::operator()
    ));
    new_tab_button->signal_clicked().connect(sigc::mem_fun(on_tab_added,
        &std::function<void()>::operator()
    ));
    close_tab_button->signal_clicked().connect(sigc::mem_fun(on_tab_closed,
        &std::function<void()>::operator()
    ));
    stack->property_visible_child().signal_changed().connect(sigc::mem_fun(on_active_tab_changed,
        &std::function<void()>::operator()
    ));
    history_previous_button->signal_clicked().connect(sigc::mem_fun(on_history_previous,
        &std::function<void()>::operator()
    ));
    history_next_button->signal_clicked().connect(sigc::mem_fun(on_history_next,
        &std::function<void()>::operator()
    ));
    webview_refresh_button->signal_clicked().connect(sigc::mem_fun(on_webview_refresh_button_clicked,
        &std::function<void()>::operator()
    ));
    webview_find_button->signal_clicked().connect(sigc::mem_fun(on_webview_find_button_clicked,
        &std::function<void()>::operator()
    ));
    webview_find_bar->property_search_mode_enabled().signal_changed().connect(sigc::mem_fun(
        on_webview_find_bar_state_changed, &std::function<void()>::operator()
    ));
    webview_find->signal_changed().connect(sigc::mem_fun(on_webview_find_text_changed,
        &std::function<void()>::operator()
    ));
    webview_find_previous_button->signal_clicked().connect(sigc::mem_fun(on_webview_find_previous,
        &std::function<void()>::operator()
    ));
    webview_find_next_button->signal_clicked().connect(sigc::mem_fun(on_webview_find_next,
        &std::function<void()>::operator()
    ));
    quit_button->signal_clicked().connect(sigc::mem_fun(on_quit_button_clicked,
        &std::function<void()>::operator()
    ));
    search_entry->signal_changed().connect(sigc::mem_fun(on_search_changed,
        &std::function<void()>::operator()
    ));
    preferences_documentation_search_path->property_has_focus().signal_changed().connect(sigc::mem_fun(
        on_preferences_documentation_search_path_unfocused, &std::function<void()>::operator()
    ));
    preferences_use_system_fonts->property_state().signal_changed().connect(sigc::mem_fun(
        on_preferences_use_system_fonts_changed, &std::function<void()>::operator()
    ));
    preferences_default_font->signal_font_set().connect(sigc::mem_fun(
        on_preferences_default_font_changed, &std::function<void()>::operator()
    ));
    preferences_monospace_font->signal_font_set().connect(sigc::mem_fun(
        on_preferences_monospace_font_changed, &std::function<void()>::operator()
    ));
    preferences_extension_search_path_expander->property_expanded().signal_changed().connect(sigc::mem_fun(
        on_preferences_extension_search_path_expander_state_changed, &std::function<void()>::operator()
    ));
    preferences_extension_search_path->property_has_focus().signal_changed().connect(sigc::mem_fun(
        on_preferences_extension_search_path_unfocused, &std::function<void()>::operator()
    ));
    preferences_extension_list->signal_row_activated().connect(sigc::mem_fun(
        on_preferences_extension_enable_toggled,
        &std::function<void(const Gtk::TreeModel::Path&, Gtk::TreeView::Column*)>::operator()
    ));
    preferences_close_button->signal_clicked().connect(sigc::mem_fun(
        on_preferences_close_button_clicked,
        &std::function<void()>::operator()
    ));

    // Manually trigger tab added handler, which will create the initial tab
    on_tab_added();

    build_tree = [&](const docview::doc_tree_node* node, Gtk::TreeStore::iterator row) -> void
    {

        // Set value of columns
        (*row)[sidebar_column_title] = node->title;
        (*row)[sidebar_column_node] = node;

        // Do the same for all children
        for (auto& child : node->children)
            build_tree(child, sidebar_contents->append(row->children()));
    };

    preferences_extension_search_path->set_buffer(preferences_extension_search_path_buffer);
    preferences_documentation_search_path->set_buffer(preferences_documentation_search_path_buffer);

    Gtk::TreeModel::ColumnRecord extension_list_columns;
    extension_list_columns.add(extension_list_column_name);
    extension_list_columns.add(extension_list_column_enabled);
    extension_list_columns.add(extension_list_column_path);
    extension_list_contents = Gtk::ListStore::create(extension_list_columns);
    preferences_extension_list->set_model(extension_list_contents);
    preferences_extension_list->append_column("Name", extension_list_column_name);
    preferences_extension_list->append_column_editable("Enable", extension_list_column_enabled);
    preferences_extension_list->get_column(0)->set_expand(true);

    // Setup the sidebar
    Gtk::TreeModel::ColumnRecord sidebar_columns;
    sidebar_columns.add(sidebar_column_title);
    sidebar_columns.add(sidebar_column_node);
    sidebar_contents = Gtk::TreeStore::create(sidebar_columns);
    sidebar_search_results = Gtk::ListStore::create(sidebar_columns);
    sidebar_tree->set_model(sidebar_contents);
    sidebar_tree->append_column("title", sidebar_column_title);

    // Trigger the following event, which will fill the extension list and sidebar
    on_preferences_extension_search_path_unfocused();

    // Change fonts
    on_preferences_use_system_fonts_changed();

    // Finally, show the window to user
    window->show_all_children();
    about_dialog->show_all_children();

    // Store status in a variable, as we might need to do some destruction tasks manually
    int status = app->run(*window);

    // Save max search result setting
    config.set_value(
        {"preferences", "interface", "search", "max_results"},
        std::to_string(preferences_max_search_results->get_value_as_int())
    );

    // Return status of application
    return status;
}
