#pragma once
#include "Windows.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>


extern HANDLE hConsole;



void PrintText(const char* text, WORD color = 15);
extern bool getting_input;



class VirtualKeyboard : public QMainWindow
{

public:
    VirtualKeyboard()
    {
        setWindowTitle("Virtual Keyboard");
        CreateKeyboardUI();
    }


    void SendKeyboardInput();


private:
    QLineEdit* virtual_input;
    QLabel* label_instructions;




    void CreateKeyboardUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);


        QVBoxLayout* main_layout = new QVBoxLayout(centralWidget);

        virtual_input = new QLineEdit(this);
        virtual_input->setReadOnly(true);
        virtual_input->setPlaceholderText("Your input");
        main_layout->addWidget(virtual_input);


        QGridLayout* layout = new QGridLayout();
        main_layout->addLayout(layout);


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

        QStringList KeyLayout = LowerKeyLayout;



        // counting rows and columns
        int row = 0, column = 0;
        for (const QString& text : KeyLayout)
        {

            QPushButton* button = new QPushButton(text);

            connect(button, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->insert(text);
                });

            layout->addWidget(button, row, column);
            column++;


            if (column == 10)
            {
                column = 0;
                row++;
            }
        }


        // Creating the last row of buttons
        QPushButton* button_back = new QPushButton("Backspace");
        QPushButton* button_clear = new QPushButton("Clear");
        QPushButton* button_left = new QPushButton("<");
        QPushButton* button_right = new QPushButton(">");

        connect(button_back, &QPushButton::clicked, this, [=]()
            {
                virtual_input->backspace();
            });
        column = 3;
        row++;
        layout->addWidget(button_back, row, column);
        connect(button_clear, &QPushButton::clicked, this, [=]()
            {
                virtual_input->clear();
            });
        column++;
        layout->addWidget(button_clear, row, column);

        connect(button_left, &QPushButton::clicked, this, [=]()
            {
                virtual_input->cursorBackward(true, 1);
            });
        column++;
        layout->addWidget(button_left, row, column);

        connect(button_right, &QPushButton::clicked, this, [=]()
            {
                virtual_input->cursorForward(true, 1);
            });
        column++;
        layout->addWidget(button_right, row, column);



        label_instructions = new QLabel("Instructions: Click on the keys to input text", this);
        main_layout->addWidget(label_instructions);
    }

};











