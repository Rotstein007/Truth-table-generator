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

class TruthTableApp : public Gtk::ApplicationWindow {
public:
    TruthTableApp() {
        set_title("Wahrheitstabelle Generator");

        // Layout
        vbox.set_spacing(10);
        vbox.set_margin(10);
        set_child(vbox);

        // Header Bar mit Info-Button
        set_titlebar(header_bar);
        header_bar.set_title("Wahrheitstabelle Generator");
        header_bar.set_show_close_button(true);

        // Info-Button
        info_button.set_icon_name("dialog-information-symbolic");
        info_button.signal_clicked().connect(sigc::mem_fun(*this, &TruthTableApp::on_info_button_clicked));
        header_bar.pack_end(info_button);

        // Eingabefelder und Button
        hbox.set_spacing(10);
        hbox.append(entry);
        entry.set_placeholder_text("Wörter eingeben (getrennt durch Komma)");

        hbox.append(generate_button);
        generate_button.set_label("Generiere Tabelle");
        generate_button.get_style_context()->add_class("suggested-action"); // Grüner Button
        generate_button.signal_clicked().connect(sigc::mem_fun(*this, &TruthTableApp::on_generate_button_clicked));

        vbox.append(hbox);

        // Formel Eingabefeld
        hbox_formula.set_spacing(10);
        hbox_formula.append(formula_entry);
        formula_entry.set_placeholder_text("Formel eingeben (z. B. A & B)");

        vbox.append(hbox_formula);

        // Umschalt-Button für die Reihenfolge
        toggle_order_button.set_label("Reihenfolge umschalten");
        toggle_order_button.signal_clicked().connect(sigc::mem_fun(*this, &TruthTableApp::on_toggle_order_button_clicked));
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
            // Fehlerdialog anzeigen
            auto dialog = Gtk::MessageDialog(*this, "Bitte geben Sie Wörter ein!", false, Gtk::MessageType::ERROR);
            dialog.set_modal(true);
            dialog.show();
            return;
        }

        // Formel aus dem Textfeld
        auto formula_text = formula_entry.get_text();

        // Generierung der Wahrheitstabelle
        try {
            generate_truth_table(variables, formula_text);
        } catch (const std::exception& e) {
            auto dialog = Gtk::MessageDialog(*this, e.what(), false, Gtk::MessageType::ERROR);
            dialog.set_modal(true);
            dialog.show();
        }
    }

    // Event-Handler für den Umschalt-Button
    void on_toggle_order_button_clicked() {
        mirrored_order = !mirrored_order;
        auto input_text = entry.get_text();
        auto variables = split(input_text, ',');

        if (!variables.empty()) {
            generate_truth_table(variables, formula_entry.get_text());
        }
    }

    // Event-Handler für den Info-Button
    void on_info_button_clicked() {
        auto dialog = Gtk::MessageDialog(
            *this,
            "Wahrheitstabelle Generator\n\n"
            "Erstellt Wahrheitstabellen basierend auf Variablen und logischen Formeln.\n"
            "Unterstützte Operatoren: & (UND), | (ODER), ! (NICHT)\n\n"
            "Version: 1.0\n"
            "Autor: Entwicklerteam",
            false,
            Gtk::MessageType::INFO
        );
        dialog.set_modal(true);
        dialog.show();
    }

    // Generierung der Wahrheitstabelle
    void generate_truth_table(const std::vector<std::string>& variables, const std::string& formula) {
        size_t rows = std::pow(2, variables.size());

        // Spaltenmodell erstellen
        Gtk::TreeModelColumnRecord columns;
        std::vector<Gtk::TreeModelColumn<std::string>> col_vars;
        for (const auto& var : variables) {
            col_vars.emplace_back();
            columns.add(col_vars.back());
        }

        // Zusätzliche Spalte für die Formel
        Gtk::TreeModelColumn<std::string> col_formula;
        bool has_formula = !formula.empty();
        if (has_formula) {
            columns.add(col_formula);
        }

        list_store = Gtk::ListStore::create(columns);
        tree_view.set_model(list_store);

        // Tabelle füllen
        for (size_t i = 0; i < rows; ++i) {
            size_t row_index = mirrored_order ? rows - i - 1 : i; // Reihenfolge umkehren, falls gespiegelt
            auto row = *(list_store->append());
            std::bitset<32> binary(row_index); // Bis zu 32 Variablen
            size_t var_index = 0;

            // Variablen-Spalten füllen
            for (const auto& var : variables) {
                row[col_vars[var_index]] = binary[variables.size() - var_index - 1] ? "W" : "F";
                ++var_index;
            }

            // Formel-Spalte berechnen, falls vorhanden
            if (has_formula) {
                try {
                    row[col_formula] = evaluate_formula(formula, variables, binary) ? "W" : "F";
                } catch (const std::exception&) {
                    row[col_formula] = "Fehler";
                }
            }
        }

        // Spalten in TreeView hinzufügen
        tree_view.remove_all_columns();
        size_t var_index = 0;
        for (const auto& var : variables) {
            tree_view.append_column(var, col_vars[var_index]);
            ++var_index;
        }
        if (has_formula) {
            tree_view.append_column(formula, col_formula);
        }

        // Fenstergröße anpassen
        int row_height = 30; // Höhe einer Zeile
        int header_height = 40; // Höhe der Kopfzeile
        int table_height = std::min<int>(rows * row_height + header_height, 600); // Maximale Höhe von 600px

        // ScrolledWindow-Größe anpassen
        scrolled_window.set_min_content_height(table_height);
        scrolled_window.set_min_content_width(variables.size() * 100 + (has_formula ? 150 : 0)); // Spaltenbreite

        // Größenrichtlinien des Fensters festlegen
        set_default_size(scrolled_window.get_min_content_width(), table_height + 60);
        set_size_request(scrolled_window.get_min_content_width(), table_height + 60);
    }

    // Logische Formel auswerten
    bool evaluate_formula(const std::string& formula, const std::vector<std::string>& variables, const std::bitset<32>& binary) {
        std::map<std::string, bool> values;
        for (size_t i = 0; i < variables.size(); ++i) {
            values[variables[i]] = binary[variables.size() - i - 1];
        }

        // Ersetzen der Variablen durch ihre Werte
        std::string eval_formula = formula;
        for (const auto& [var, value] : values) {
            auto var_value = value ? "1" : "0";
            eval_formula = std::regex_replace(eval_formula, std::regex("\\b" + var + "\\b"), var_value);
        }

        // Logischen Ausdruck auswerten (vereinfacht, nur für &, |, !)
        std::stack<bool> stack;
        for (char c : eval_formula) {
            if (c == '1') {
                stack.push(true);
            } else if (c == '0') {
                stack.push(false);
            } else if (c == '&') {
                if (stack.size() < 2) throw std::runtime_error("Fehler: Ungültige Formel!");
                auto b = stack.top(); stack.pop();
                auto a = stack.top(); stack.pop();
                stack.push(a && b);
            } else if (c == '|') {
                if (stack.size() < 2) throw std::runtime_error("Fehler: Ungültige Formel!");
                auto b = stack.top(); stack.pop();
                auto a = stack.top(); stack.pop();
                stack.push(a || b);
            } else if (c == '!') {
                if (stack.empty()) throw std::runtime_error("Fehler: Ungültige Formel!");
                auto a = stack.top(); stack.pop();
                stack.push(!a);
            }
        }

        if (stack.size() != 1) {
            throw std::runtime_error("Fehler bei der Auswertung der Formel.");
        }
        return stack.top();
    }

    // Hilfsfunktion zum Aufteilen des Strings
    static std::vector<std::string> split(const std::string& text, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream(text);
        while (std::getline(token_stream, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.example.truth_table");
    TruthTableApp window;

    return app->make_window_and_run<TruthTableApp>(argc, argv);
}
