#include <gtkmm.h>
#include <vector>
#include <string>
#include <cmath>
#include <bitset>
#include <sstream>
#include <set>
#include <regex>
#include <stdexcept>
#include <stack>
#include <map>

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow() {
        set_title("Wahrheitstabelle Generator");

        // Layout
        vbox.set_spacing(10);
        vbox.set_margin(10);
        set_child(vbox);

        // Header Bar mit Info-Button
        set_titlebar(header_bar);
        auto title_label = Gtk::make_managed<Gtk::Label>("Wahrheitstabelle Generator");
        header_bar.set_title_widget(*title_label);
        header_bar.set_show_title_buttons(true);

        // Info-Button
        info_button.set_icon_name("dialog-information-symbolic");
        info_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_info_button_clicked));
        header_bar.pack_end(info_button);

        // Eingabefelder und Button
        hbox.set_spacing(10);
        hbox.append(entry);
        entry.set_placeholder_text("Variablen eingeben (getrennt durch Komma)");

        hbox.append(generate_button);
        generate_button.set_label("Generiere Tabelle");
        generate_button.get_style_context()->add_class("suggested-action"); // Grüner Button
        generate_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_generate_button_clicked));

        vbox.append(hbox);

        // Formel Eingabefeld
        hbox_formula.set_spacing(10);
        hbox_formula.append(formula_entry);
        formula_entry.set_placeholder_text("Formel eingeben (z. B. A & B)");

        vbox.append(hbox_formula);

        // Umschalt-Button für die Reihenfolge
        toggle_order_button.set_label("Reihenfolge umschalten");
        toggle_order_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_toggle_order_button_clicked));
        vbox.append(toggle_order_button);

        // TreeView für die Tabelle
        scrolled_window.set_child(tree_view);
        vbox.append(scrolled_window);
    }

private:
    Gtk::Box vbox{Gtk::Orientation::VERTICAL};
    Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
    Gtk::Box hbox_formula{Gtk::Orientation::HORIZONTAL};
    Gtk::Entry entry;
    Gtk::Entry formula_entry;
    Gtk::Button generate_button;
    Gtk::Button toggle_order_button;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView tree_view;
    Gtk::HeaderBar header_bar;
    Gtk::Button info_button;
    Glib::RefPtr<Gtk::ListStore> list_store;

    bool mirrored_order = false; // Standardmäßig normale Reihenfolge

    // Event-Handler für den "Generiere Tabelle"-Button
    void on_generate_button_clicked() {
        auto input_text = entry.get_text();
        auto variables = split(input_text, ',');

        if (variables.empty()) {
            show_error("Bitte geben Sie Variablen ein!");
            return;
        }

        auto formula_text = formula_entry.get_text();

        try {
            generate_truth_table(variables, formula_text);
        } catch (const std::exception& e) {
            show_error(e.what());
        }
    }

    void on_toggle_order_button_clicked() {
        mirrored_order = !mirrored_order;
        auto input_text = entry.get_text();
        auto variables = split(input_text, ',');

        if (!variables.empty()) {
            generate_truth_table(variables, formula_entry.get_text());
        }
    }

    void on_info_button_clicked() {
        Gtk::AboutDialog about_dialog;
        about_dialog.set_program_name("Wahrheitstabelle Generator");
        about_dialog.set_version("1.0");
        about_dialog.set_comments("Erstellt Wahrheitstabellen basierend auf Variablen und logischen Formeln.");
        about_dialog.set_license("GPL 3.0");
        about_dialog.set_authors({"Entwicklerteam"});
        about_dialog.set_logo_icon_name("dialog-information-symbolic");

        // Open the dialog without using 'run()'
        about_dialog.present();
    }

    void generate_truth_table(const std::vector<std::string>& variables, const std::string& formula) {
        size_t rows = std::pow(2, variables.size());

        Gtk::TreeModelColumnRecord columns;
        std::vector<Gtk::TreeModelColumn<std::string>> col_vars;
        for (size_t i = 0; i < variables.size(); ++i) {
            col_vars.emplace_back();
            columns.add(col_vars.back());
        }

        Gtk::TreeModelColumn<std::string> col_formula;
        bool has_formula = !formula.empty();
        if (has_formula) {
            columns.add(col_formula);
        }

        list_store = Gtk::ListStore::create(columns);
        tree_view.set_model(list_store);

        for (size_t i = 0; i < rows; ++i) {
            size_t row_index = mirrored_order ? rows - i - 1 : i;
            auto row = *(list_store->append());
            std::bitset<32> binary(row_index);
            size_t var_index = 0;

            for (const auto& var : variables) {
                row[col_vars[var_index]] = binary[variables.size() - var_index - 1] ? "W" : "F";
                ++var_index;
            }

            if (has_formula) {
                try {
                    row[col_formula] = evaluate_formula(formula, variables, binary) ? "W" : "F";
                } catch (const std::exception&) {
                    row[col_formula] = "Fehler";
                }
            }
        }

        tree_view.remove_all_columns();
        size_t var_index = 0;
        for (const auto& var : variables) {
            tree_view.append_column(var, col_vars[var_index]);
            ++var_index;
        }
        if (has_formula) {
            tree_view.append_column("Formel", col_formula);
        }

        int row_height = 30;
        int header_height = 40;
        int table_height = std::min<int>(rows * row_height + header_height, 600);

        scrolled_window.set_min_content_height(table_height);
        scrolled_window.set_min_content_width(variables.size() * 100 + (has_formula ? 150 : 0));

        set_default_size(scrolled_window.get_min_content_width(), table_height + 60);
        set_size_request(scrolled_window.get_min_content_width(), table_height + 60);
    }

    bool evaluate_formula(const std::string& formula, const std::vector<std::string>& variables, const std::bitset<32>& binary) {
        // Variablen ersetzen
        std::map<std::string, bool> values;
        for (size_t i = 0; i < variables.size(); ++i) {
            values[variables[i]] = binary[variables.size() - i - 1];
        }

        // Ersetzen der Variablen in der Formel
        std::string eval_formula = formula;
        for (const auto& [var, value] : values) {
            auto var_value = value ? "1" : "0";
            std::regex var_regex("\\b" + var + "\\b");
            eval_formula = std::regex_replace(eval_formula, var_regex, var_value);
        }

        // Klammerauswertung
        std::stack<char> operators;
        std::stack<bool> operands;

        auto apply_operator = [](char op, bool a, bool b) {
            switch (op) {
                case '&': return a && b;
                case '|': return a || b;
                default: throw std::runtime_error("Unbekannter Operator");
            }
        };

        auto apply_not = [](bool a) {
            return !a;
        };

        for (size_t i = 0; i < eval_formula.size(); ++i) {
            char c = eval_formula[i];

            if (c == '1') {
                operands.push(true);
            } else if (c == '0') {
                operands.push(false);
            } else if (c == '!') {
                if (operands.empty()) throw std::runtime_error("Fehler: Ungültiger Ausdruck");
                bool top = operands.top();
                operands.pop();
                operands.push(apply_not(top));
            } else if (c == '&' || c == '|') {
                operators.push(c);
            } else if (c == ')') {
                if (operators.empty()) throw std::runtime_error("Fehler: Ungültiger Ausdruck");
                char op = operators.top();
                operators.pop();
                bool b = operands.top();
                operands.pop();
                bool a = operands.top();
                operands.pop();
                operands.push(apply_operator(op, a, b));
            }
        }

        return operands.top();
    }

    void show_error(const std::string& message) {
        Gtk::MessageDialog dialog(*this, "Fehler", false, Gtk::MessageType::ERROR, Gtk::ButtonsType::OK);
        dialog.set_secondary_text(message);

        // Replace dialog.run() with dialog.show() in GTK4
        dialog.present();
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : str) {
            if (c == delimiter) {
                tokens.push_back(token);
                token.clear();
            } else {
                token += c;
            }
        }
        if (!token.empty()) tokens.push_back(token);
        return tokens;
    }
};

class TruthTableApp : public Gtk::Application {
protected:
    void on_startup() override {
        Gtk::Application::on_startup();

        // Create the window
        window = new MainWindow();

        // Set the window as the main window of the application
        add_window(*window);
    }

    void on_activate() override {
        window->present();
    }

private:
    MainWindow* window;
};

int main(int argc, char* argv[]) {
    // Create the application object
    auto app = Gtk::Application::create("org.gtk.example");
    auto truth_table_app = new TruthTableApp();

    // Run the application with argc and argv
    return truth_table_app->run(argc, argv);  // Exactly 2 arguments
}