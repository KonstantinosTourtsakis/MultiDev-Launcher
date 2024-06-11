#pragma once
#include "Windows.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>

#include "Controller.h"


extern int screen_width;
extern int screen_height;
extern bool getting_input;
extern int current_row, current_column;




int PercentToWidth(const double percentage);
int PercentToHeight(const double percentage);









class VirtualKeyboard : public QMainWindow
{

public:
    VirtualKeyboard()
    {
        setWindowTitle("Virtual Keyboard");
        virtual_input = new QLineEdit(this);
        virtual_input->setReadOnly(true);
        virtual_input->setPlaceholderText("Your input");
        CreateKeyboardUI();
    }


    void SendKeyboardInput();


private:
    QLineEdit* virtual_input;
    QLabel* label_instructions;
    QGridLayout* layout_keyboard;
    QString current_char = "h";
    bool caps_lock = false;




    void CreateKeyboardUI()
    {
        
        
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QFont font("Arial", 18);

        QVBoxLayout* main_layout = new QVBoxLayout(centralWidget);
        virtual_input->setFixedSize(PercentToWidth(100.00), PercentToHeight(3.00));
        virtual_input->setFont(font);

        main_layout->addWidget(virtual_input);

        layout_keyboard = new QGridLayout();
        
        main_layout->addLayout(layout_keyboard);


        const QStringList UpperKeyLayout
        {
            "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
            "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
            "A", "S", "D", "F", "G", "H", "J", "K", "L", "_",
            "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",
        };


        const QStringList LowerKeyLayout
        {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
            "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
            "a", "s", "d", "f", "g", "h", "j", "k", "l", "-",
            "z", "x", "c", "v", "b", "n", "m", ",", ".", "/",
        };

        QStringList KeyLayout = caps_lock ? UpperKeyLayout : LowerKeyLayout;

        
        
        // counting rows and columns
        int row = 0, column = 0;
        for (const QString& text : KeyLayout)
        {
            QPushButton* button = new QPushButton(text);
            button->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button->setFont(font);
            
            
            if (row == current_row && column == current_column && getting_input)
            {
                button->setFixedSize(PercentToWidth(1.60), PercentToHeight(2.24));
                current_char = text;
            }

            connect(button, &QPushButton::pressed, this, [=]()
                {
                    virtual_input->insert(text);
                });

            layout_keyboard->addWidget(button, row, column);

            if (row == 2 && column == 5)
            {
                button->setChecked(true);
            }
            column++;


            if (column == 10)
            {
                column = 0;
                row++;
            }


        }
        
        


        

        if (getting_input)
        {
            label_instructions = new QLabel("A: type key   Y: insert space   X: Backspace   Start: send input   Left Stick: Caps lock", this);
            //setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
        }
        else
        {   
            label_instructions = new QLabel("Click on the keys and then use the copy button to save input to clipboard", this);


            // Creating the last row of buttons
            QPushButton* button_copy = new QPushButton("Copy Input");
            QPushButton* button_caps = new QPushButton("Caps Lock");
            QPushButton* button_space = new QPushButton("Space");
            QPushButton* button_back = new QPushButton("Backspace");
            QPushButton* button_clear = new QPushButton("Clear");
            QPushButton* button_left = new QPushButton("<");
            QPushButton* button_right = new QPushButton(">");


            button_copy->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_copy->setFont(QFont("Arial", 12));

            button_caps->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_caps->setFont(QFont("Arial", 12));

            button_space->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_space->setFont(font);

            button_back->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_back->setFont(QFont("Arial", 12));

            button_clear->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_clear->setFont(font);

            button_left->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_left->setFont(font);

            button_right->setFixedSize(PercentToWidth(2.60), PercentToHeight(3.24));
            button_right->setFont(font);


            connect(button_copy, &QPushButton::clicked, this, [=]()
                {
                    Beep(200, 200);
                    QClipboard* clipboard = QGuiApplication::clipboard();
                    clipboard->setText(virtual_input->text());
                });

            connect(button_caps, &QPushButton::clicked, this, [=]()
                {
                    caps_lock = !caps_lock;
                    CreateKeyboardUI();
                });

            connect(button_space, &QPushButton::clicked, this, [=]()
                {
                    Beep(200, 200);
                    virtual_input->insert(" ");
                });

            connect(button_back, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->backspace();
                });

            connect(button_clear, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->clear();
                });

            connect(button_left, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->cursorBackward(true, 1);
                });

            connect(button_right, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->cursorForward(true, 1);
                });

            column = 0;
            row++;



            column++;
            layout_keyboard->addWidget(button_caps, row, column);
            column++;
            layout_keyboard->addWidget(button_space, row, column);
            column++;
            layout_keyboard->addWidget(button_back, row, column);
            column++;
            layout_keyboard->addWidget(button_clear, row, column);
            column++;
            layout_keyboard->addWidget(button_left, row, column);
            column++;
            layout_keyboard->addWidget(button_right, row, column);
            column++;
            layout_keyboard->addWidget(button_copy, row, column);
        }
        
        label_instructions->setFont(font);
        main_layout->addWidget(label_instructions, 0, Qt::AlignCenter);

        setFixedSize(PercentToWidth(33.33), PercentToHeight(33.33));

        
    }


};










