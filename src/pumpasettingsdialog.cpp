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

//------------------------------------------------------------------------------

PumpaSettingsDialog::PumpaSettingsDialog(QSettings* settings,
                                         QWidget* parent):
  QDialog(parent),
  s(settings)
{
  m_layout = new QVBoxLayout;

  m_layout->addWidget(new QLabel("Currently logged in as sazius@io.saz.im."));

  // [Change account]

  QLabel* acctInfoLabel = 
    new QLabel("Clicking \"Change account\" will run the "
               "authentication setup again for a new pump.io "
               "account. This will remove the current login "
               "credentials since Pumpa only supports one "
               "account at a time.");
  acctInfoLabel->setWordWrap(true);
  m_layout->addWidget(acctInfoLabel);

  m_formLayout = new QFormLayout;

  m_updateTimeSpinBox = new QSpinBox(this);
  m_updateTimeSpinBox->setMinimum(1);
  m_updateTimeSpinBox->setMaximum(30);

  m_formLayout->addRow("Update interval (in minutes):",
                       m_updateTimeSpinBox);

  m_useIconCheckBox = new QCheckBox("Use icon in system tray", this);
  m_formLayout->addRow(m_useIconCheckBox);

  QStringList timelineList;
  timelineList << "Never"
               << "Direct only"
               << "Direct or mention"
               << "Direct, mention or inbox"
               << "Anything";

  m_highlightComboBox = new QComboBox(this);
  m_highlightComboBox->addItems(timelineList);
  m_formLayout->addRow("Highlight tray icon on:", m_highlightComboBox);

  m_popupComboBox = new QComboBox(this);
  m_popupComboBox->addItems(timelineList);
  m_formLayout->addRow("Popup notification on:", m_popupComboBox);
  
  m_layout->addLayout(m_formLayout);

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Ok);
  connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  m_layout->addWidget(m_buttonBox);

  setLayout(m_layout);
}

