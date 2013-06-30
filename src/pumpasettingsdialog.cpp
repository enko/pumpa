/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pumpasettingsdialog.h"
#include "util.h"
#include "pumpa_defines.h"

//------------------------------------------------------------------------------

PumpaSettingsDialog::PumpaSettingsDialog(PumpaSettings* settings,
                                         QWidget* parent):
  QDialog(parent),
  s(settings)
{
  m_layout = new QVBoxLayout;

  // Account
  QGroupBox* accountGroupBox = new QGroupBox("Account");
  QVBoxLayout* accountLayout = new QVBoxLayout;

  m_currentAccountLabel = new QLabel("Not logged in currently.", this);
  accountLayout->addWidget(m_currentAccountLabel);

  m_authButton = new QPushButton("Change account", this);
  connect(m_authButton, SIGNAL(clicked()), this, SLOT(onAuthButtonClicked()));
  accountLayout->addWidget(m_authButton);

  QLabel* acctInfoLabel = 
    new QLabel("Clicking \"Change account\" will run the "
               "authentication setup again for a new pump.io "
               "account. This will remove the current login "
               "credentials since Pumpa only supports one "
               "account at a time.");
  acctInfoLabel->setWordWrap(true);
  accountLayout->addWidget(acctInfoLabel);

  accountGroupBox->setLayout(accountLayout);
  m_layout->addWidget(accountGroupBox);

  // User interface
  QGroupBox* uiGroupBox = new QGroupBox("Interface");
  QFormLayout* uiLayout = new QFormLayout;

  m_updateTimeSpinBox = new QSpinBox(this);
  m_updateTimeSpinBox->setMinimum(1);
  m_updateTimeSpinBox->setMaximum(30);

  uiLayout->addRow("Update interval (in minutes):",
                       m_updateTimeSpinBox);

  m_useIconCheckBox = new QCheckBox("Use icon in system tray", this);
  uiLayout->addRow(m_useIconCheckBox);

  uiGroupBox->setLayout(uiLayout);
  m_layout->addWidget(uiGroupBox);
  
  // Notifications
  QGroupBox* notifyGroupBox = new QGroupBox("Notifications");
  QFormLayout* notifyLayout = new QFormLayout;

  QStringList timelineList;
  timelineList << "Never"
               << "Direct only"
               << "Direct or mention"
               << "Direct, mention or inbox"
               << "Anything";

  m_highlightComboBox = new QComboBox(this);
  m_highlightComboBox->addItems(timelineList);
  notifyLayout->addRow("Highlight tray icon on:", m_highlightComboBox);

  m_popupComboBox = new QComboBox(this);
  m_popupComboBox->addItems(timelineList);
  notifyLayout->addRow("Popup notification on:", m_popupComboBox);
  
  notifyGroupBox->setLayout(notifyLayout);
  m_layout->addWidget(notifyGroupBox);

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(onOKClicked()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  m_layout->addWidget(m_buttonBox);

  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void PumpaSettingsDialog::onAuthButtonClicked() {
  accept();
  emit newAccount();
}

//------------------------------------------------------------------------------

void PumpaSettingsDialog::setVisible(bool visible) {
  if (visible)
    updateUI();
  QDialog::setVisible(visible);
}

//------------------------------------------------------------------------------

int PumpaSettingsDialog::comboIndexConverter(int ci, bool backwards) {
  static QList<int> comboToFeeds;
  if (comboToFeeds.isEmpty()) {
    comboToFeeds << 0
                 << FEED_DIRECT
                 << (FEED_DIRECT | FEED_MENTIONS)
                 << (FEED_DIRECT | FEED_MENTIONS | FEED_INBOX)
                 << (FEED_DIRECT | FEED_MENTIONS | FEED_INBOX | FEED_MEANWHILE);
  }

  if (backwards) {
    int ret = comboToFeeds.indexOf(ci);
    return ret == -1 ? 0 : ret;
  }

  return ci < comboToFeeds.size() ? comboToFeeds[ci] : 0;
}

//------------------------------------------------------------------------------

void PumpaSettingsDialog::updateUI() {
  QString accountId = siteUrlToAccountId(s->userName(), s->siteUrl());
  m_currentAccountLabel->setText(QString("Currently logged in as %1.").
                                 arg(accountId));
  
  m_updateTimeSpinBox->setValue(s->reloadTime());

  m_useIconCheckBox->setChecked(s->useTrayIcon());

  m_highlightComboBox->
    setCurrentIndex(feedIntToComboIndex(s->highlightFeeds()));

  m_popupComboBox->
    setCurrentIndex(feedIntToComboIndex(s->popupFeeds()));
}

//------------------------------------------------------------------------------

void PumpaSettingsDialog::onOKClicked() {
  s->reloadTime(m_updateTimeSpinBox->value());
  s->useTrayIcon(m_useIconCheckBox->isChecked());

  s->highlightFeeds(comboIndexToFeedInt(m_highlightComboBox->currentIndex()));
  s->popupFeeds(comboIndexToFeedInt(m_popupComboBox->currentIndex()));

  emit accepted();
}
