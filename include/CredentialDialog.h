#ifndef CREDENTIALDIALOG_H
#define CREDENTIALDIALOG_H

#include <QDialog>
#include "Credential.h"

class QLineEdit;
class QComboBox;
class QTextEdit;
class QCheckBox;
class QPushButton;

// One dialog handles both "Add Credential" and "Edit Credential".
// Construct with no argument for Add, or pass an existing Credential
// to pre-fill the fields for Edit.
class CredentialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CredentialDialog(QWidget *parent = nullptr);
    explicit CredentialDialog(const Credential &existing, QWidget *parent = nullptr);

    Credential credential() const;

private slots:
    void onSaveClicked();
    void togglePasswordVisibility();
    void onGeneratePasswordClicked();

private:
    void setupUi();
    void populateFromCredential(const Credential &existing);

    QLineEdit *m_websiteEdit;
    QLineEdit *m_urlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_toggleVisibilityButton;
    QPushButton *m_generateButton;
    QComboBox *m_categoryCombo;
    QTextEdit *m_notesEdit;
    QCheckBox *m_favoriteCheck;
    QPushButton *m_saveButton;

    Credential m_credential;
    bool m_isEditMode;
};

#endif // CREDENTIALDIALOG_H
