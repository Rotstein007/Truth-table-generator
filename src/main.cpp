#include <gtkmm.h>
#include <vector>
#include <string>
#include <cmath>
#include <bitset>
#include <sstream>
#include <set>
#include <regex>

class TruthTableApp : public Gtk::ApplicationWindow {
public:
    TruthTableApp() {
        set_title("Wahrheitstabelle Generator");

        // Layout
        vbox.set_spacing(10);
        vbox.set_margin(10);
        set_child(vbox);

        // Eingabefelder und Button
        hbox.set_spacing(10);
        hbox.append(entry);
        entry.set_placeholder_text("Wörter eingeben (getrennt durch Komma)");

        hbox.append(generate_button);
        generate_button.set_label("Generiere Tabelle");
        generate_button.signal_clicked().connect(sigc::mem_fun(*this, &TruthTableApp::on_generate_button_clicked));

        vbox.append(hbox);

        // Formel Eingabefeld
        hbox_formula.set_spacing(10);
        hbox_formula.append(formula_entry);
        formula_entry.set_placeholder_text("Formel eingeben");

        vbox.append(hbox_formula);

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
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView tree_view;
    Glib::RefPtr<Gtk::ListStore> list_store;

    // Set zur Vermeidung doppelter Variablen
    std::set<std::string> unique_variables;

    // Event-Handler für den Button
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
        auto formula_variables = parse_formula(formula_text);

        // Vereinigung der Variablen aus der Eingabe und der Formel
        variables.insert(variables.end(), formula_variables.begin(), formula_variables.end());
        for (const auto& var : variables) {
            unique_variables.insert(var);  // Alle Variablen in einem Set speichern (keine Duplikate)
        }

        generate_truth_table(unique_variables);
    }

    // Generierung der Wahrheitstabelle
    void generate_truth_table(const std::set<std::string>& variables) {
        size_t rows = std::pow(2, variables.size());

        // Spaltenmodell erstellen
        Gtk::TreeModelColumnRecord columns;
        std::vector<Gtk::TreeModelColumn<std::string>> col_vars;
        for (const auto& var : variables) {
            col_vars.emplace_back();
            columns.add(col_vars.back());
        }

        list_store = Gtk::ListStore::create(columns);
        tree_view.set_model(list_store);

        // Tabelle füllen
        for (size_t i = 0; i < rows; ++i) {
            auto row = *(list_store->append());
            std::bitset<32> binary(i); // Bis zu 32 Variablen
            size_t var_index = 0;
            for (const auto& var : variables) {
                row[col_vars[var_index]] = binary[variables.size() - var_index - 1] ? "W" : "F";
                ++var_index;
            }
        }

        // Spalten in TreeView hinzufügen
        tree_view.remove_all_columns();
        size_t var_index = 0;
        for (const auto& var : variables) {
            tree_view.append_column(var, col_vars[var_index]);
            ++var_index;
        }

        // Fenstergröße anpassen
        int row_height = 30; // Höhe einer Zeile
        int header_height = 40; // Höhe der Kopfzeile
        int table_height = rows * row_height + header_height;

        // ScrolledWindow-Größe anpassen
        scrolled_window.set_min_content_height(table_height);
        scrolled_window.set_min_content_width(variables.size() * 100); // Spaltenbreite

        // Größenrichtlinien des Fensters festlegen
        set_default_size(scrolled_window.get_min_content_width(), table_height + 60);
        set_size_request(scrolled_window.get_min_content_width(), table_height + 60);
    }

    // Hilfsfunktion zum Aufteilen des Strings
    std::vector<std::string> split(const std::string& text, char delimiter) {
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

    // Formel parsen und Variablen extrahieren
    std::set<std::string> parse_formula(const std::string& formula) {
        std::set<std::string> variables;
        std::regex var_regex("[A-Za-z]+");  // Regulärer Ausdruck für Variablen (Buchstaben)
        auto words_begin = std::sregex_iterator(formula.begin(), formula.end(), var_regex);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            variables.insert(i->str());  // Alle gefundenen Variablen speichern
        }

        return variables;
    }
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.example.truthtable");
    return app->make_window_and_run<TruthTableApp>(argc, argv);
}