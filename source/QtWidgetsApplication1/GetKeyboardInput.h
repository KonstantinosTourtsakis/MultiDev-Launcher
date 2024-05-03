#pragma once
#include "Windows.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>





extern bool getting_input;



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

    bool caps_lock = false;


    void CreateKeyboardUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QFont font("Arial", 18);

        QVBoxLayout* main_layout = new QVBoxLayout(centralWidget);
        virtual_input->setFixedSize(3840, 40);
        virtual_input->setFont(font);
        
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

        QStringList KeyLayout = caps_lock ? UpperKeyLayout : LowerKeyLayout;


        /*
        int row = 0, column = 0;
        QListWidget* keys_list = new QListWidget(this);
        keys_list->setFixedSize(400, 200);
        keys_list->setIconSize(QSize(64, 64)); // Set the size of the icons

        for (const QString& text : KeyLayout)
        {


            QListWidgetItem* item = new QListWidgetItem(text);

            keys_list->addItem(item);

            
            
            
            column++;


            if (column == 10)
            {
                column = 0;
                row++;
            }

        }



        //keys_list->setViewMode(QListWidget::IconMode);
        //keys_list->setMovement(QListView::Static);
        //keys_list->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        //keys_list->setCurrentItem(keys_list->itemAt(QPoint(2, 5)));
        //keys_list->setSpacing(40);
        
        
        layout->addWidget(keys_list, 0, 0, 10, 10, Qt::AlignCenter);
        //layout->addWidget(keys_list);


        connect(keys_list, &QListWidget::itemActivated, this, &VirtualKeyboard::OnKeyClicked);
        */

        
        // counting rows and columns
        int row = 0, column = 0;
        for (const QString& text : KeyLayout)
        {

            QPushButton* button = new QPushButton(text);
            button->setFixedSize(100, 70);
            button->setFont(font);

            connect(button, &QPushButton::pressed, this, [=]()
                {
                    virtual_input->insert(text);
                });

            layout->addWidget(button, row, column);
            
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

        // Creating the last row of buttons
        QPushButton* button_back = new QPushButton("Backspace");
        QPushButton* button_clear = new QPushButton("Clear");
        QPushButton* button_left = new QPushButton("<");
        QPushButton* button_right = new QPushButton(">");

        button_back->setFixedSize(100, 70);
        button_back->setFont(font);

        button_clear->setFixedSize(100, 70);
        button_clear->setFont(font);

        button_left->setFixedSize(100, 70);
        button_left->setFont(font);

        button_right->setFixedSize(100, 70);
        button_right->setFont(font);

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
        label_instructions->setFont(font);
        main_layout->addWidget(label_instructions);
    }

private slots:
    void OnKeyClicked(QListWidgetItem* item)
    {
        if (item)
        {
            virtual_input->insert(item->text());
        }
    }

};











