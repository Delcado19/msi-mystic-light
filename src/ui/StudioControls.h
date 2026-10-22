#pragma once
#include <QString>
#include <QPushButton>
#include <QSizePolicy>

// Actions hug their labels; swatches remain expandable visual color samples.
inline void sizeStudioAction(QPushButton* button) {
    button->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
    button->setMinimumHeight(32);
}

// The visible stepper button is also its hit target; share metrics across dialogs.
inline QString compactStudioControlsStyle() {return QString::fromUtf8(R"(
            QWidget { color: #edf2f7; }
            QDialog, QScrollArea > QWidget > QWidget { background: #12151c; }
            QLabel { background: transparent; }
            QLineEdit, QComboBox { background: #242b38; color: #edf2f7; border: 1px solid #465269; border-radius: 5px; }
            QComboBox QAbstractItemView { background: #242b38; color: #edf2f7; selection-background-color: #173449; }
            QListWidget { background: #171d28; color: #edf2f7; border: 1px solid #333d4e; border-radius: 5px; }
            QListWidget::item:selected { background: #173449; color: #e6faff; }
            QListWidget#effectLayers::item:selected { background: #35294d; color: #f1e8ff; border: 1px solid #b08aff; }
            QScrollArea { border: none; background: #12151c; }
            QScrollBar:horizontal { background: #171d28; height: 12px; }
            QScrollBar::handle:horizontal { background: #526171; min-width: 32px; border-radius: 4px; }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
            QScrollBar:vertical { background: #171d28; width: 8px; }
            QScrollBar::handle:vertical { background: #526171; min-height: 30px; border-radius: 4px; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
            QPushButton { background: #242b38; color: #edf2f7; border: 1px solid #465269; border-radius: 5px; }
            QPushButton:hover { background: #303b4e; border-color: #7e9cb7; }
            QPushButton:pressed, QPushButton:checked { background: #183c50; border-color: #52d6ff; }
            QPushButton[primary="true"] { background: #52d6ff; color: #09151f; border-color: #52d6ff; font-weight: 600; }
            QPushButton[primary="true"]:hover { background: #8ce5ff; }
            QPushButton:disabled { background: #1b202b; color: #8490a2; border-color: #333d4e; }
            QComboBox { padding: 3px 8px; min-height: 22px; }
            QPushButton { padding: 3px 10px; min-height: 24px; }
            QLineEdit { padding: 3px 6px; min-height: 22px; }
            QSpinBox, QDoubleSpinBox { padding: 2px 30px 2px 6px; min-height: 26px; border: 1px solid #526171; border-radius: 5px; background: #252d39; color: #edf2f7; }
            QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPushButton:focus { border: 1px solid #52d6ff; }
            QSpinBox::up-button, QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 26px; height: 15px; border-left: 1px solid #526171; background: #303c4b; }
            QSpinBox::down-button, QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 26px; height: 15px; border-left: 1px solid #526171; background: #303c4b; }
            QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover, QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover { background: #45605e; }
            QSpinBox::up-arrow, QDoubleSpinBox::up-arrow { image: url(:/studio/up.svg); width: 9px; height: 5px; }
            QSpinBox::down-arrow, QDoubleSpinBox::down-arrow { image: url(:/studio/chevron.svg); width: 9px; height: 5px; }
            QListWidget::item { padding: 5px 7px; margin: 1px 0; }
        )");}
