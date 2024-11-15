project('package-manager', 'cpp', version: '0.1.0', default_options: ['cpp_std=c++20'])

# Standard-Dependencies für das Projekt
gtk_dep = dependency('gtkmm-4.0', version: '>=4.0')

# Definiere den Quellcode-Ordner, in diesem Fall "src"
src_dir = 'src'

# Erstelle das ausführbare Ziel aus der Datei main.cpp im src-Ordner
executable('package-manager',
           src_dir / 'main.cpp',  # Pfad zur main.cpp
           dependencies: [gtk_dep],  # Hier wird die gtkmm-4.0 Bibliothek hinzugefügt
           install: true
)
