//
// Created by ebeuque on 17/03/2021.
//

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

#include "QMemoryOperationListView.h"

QMemoryOperationListView::QMemoryOperationListView(QWidget* pParent)
	: QWidget(pParent)
{
	QBoxLayout* pMainLayout = new QVBoxLayout();
	setLayout(pMainLayout);

	QWidget* pTmpWidget;

	// Add search form
	pTmpWidget = createFilterForm(this);
	pMainLayout->addWidget(pTmpWidget);

	// Add tree view
	m_pTreeView = new QTreeView(this);
	pMainLayout->addWidget(m_pTreeView);

	// Add status bar
	pTmpWidget = createStatisticsBar(this);
	pMainLayout->addWidget(pTmpWidget);
}

QMemoryOperationListView::~QMemoryOperationListView()
{

}

QWidget* QMemoryOperationListView::createFilterForm(QWidget* pParent)
{
	QWidget* pMainWidget = new QWidget(pParent);
	pMainWidget->setContentsMargins(0, 0, 0, 0);

	QBoxLayout* pMainLayout;
	pMainLayout = new QHBoxLayout();
	pMainLayout->setContentsMargins(0, 0, 0, 0);
	pMainWidget->setLayout(pMainLayout);

	pMainLayout->addWidget(new QLabel(tr("Timestamp from:")));
	m_pTimeStampMinLineEdit = new QLineEdit();
	pMainLayout->addWidget(m_pTimeStampMinLineEdit);

	pMainLayout->addWidget(new QLabel(tr("to:")));
	m_pTimeStampMaxLineEdit = new QLineEdit();
	pMainLayout->addWidget(m_pTimeStampMaxLineEdit);

	pMainLayout->addWidget(new QLabel(tr("Thread:")));
	m_pThreadIdComboBox = new QComboBox();
	m_pThreadIdComboBox->setEditable(true);
	m_pThreadIdComboBox->setMinimumWidth(200);
	pMainLayout->addWidget(m_pThreadIdComboBox);

	pMainLayout->addWidget(new QLabel(tr("Not freed only:")));
	m_pFreedOnlyCheckBox = new QCheckBox();
	pMainLayout->addWidget(m_pFreedOnlyCheckBox);

	m_pFilterButton = new QPushButton(tr("Filter"));
	pMainLayout->addWidget(m_pFilterButton);


	pMainLayout->addStretch();

	return pMainWidget;
}

QWidget* QMemoryOperationListView::createStatisticsBar(QWidget* pParent)
{
	QWidget* pMainWidget = new QWidget(pParent);
	pMainWidget->setContentsMargins(0, 0, 0, 0);

	QGridLayout* pMainLayout;

	{
		QGridLayout* pGridLayout;
		pGridLayout = new QGridLayout();
		pGridLayout->setContentsMargins(0, 0, 0, 0);
		pMainWidget->setLayout(pGridLayout);

		// Global row
		QLabel* pTmpLabel;
		for(int i=0; i<StatusBarCol_ColCount; i++)
		{
			// Header row
			pTmpLabel = new QLabel();
			if(i == StatusBarCol_Title) {
				pTmpLabel->setText(QString());
			}else{
				pTmpLabel->setText(getColName(i));
				pTmpLabel->setAlignment(Qt::AlignCenter);
			}
			pGridLayout->addWidget(pTmpLabel, 0, i);

			// Search row
			pTmpLabel = new QLabel();
			if(i == StatusBarCol_Title) {
				pTmpLabel->setText(tr("Filter:"));
			}else{
				pTmpLabel->setText("0");
				pTmpLabel->setAlignment(Qt::AlignCenter);
			}
			m_listStatusRow1.append(pTmpLabel);
			pGridLayout->addWidget(pTmpLabel, 1, i);

			// Global row
			pTmpLabel = new QLabel();
			if(i == StatusBarCol_Title) {
				pTmpLabel->setText(tr("Global:"));
			}else{
				pTmpLabel->setText("0");
				pTmpLabel->setAlignment(Qt::AlignCenter);
			}
			m_listStatusRow2.append(pTmpLabel);
			pGridLayout->addWidget(pTmpLabel, 2, i);
		}
	}

	return pMainWidget;
}

QLineEdit* QMemoryOperationListView::getTimeStampMinLineEdit() const
{
	return m_pTimeStampMinLineEdit;
}

QLineEdit* QMemoryOperationListView::getTimeStampMaxLineEdit() const
{
	return m_pTimeStampMaxLineEdit;
}

QCheckBox* QMemoryOperationListView::getNotFreeOnlyCheckBox() const
{
	return m_pFreedOnlyCheckBox;
}

QComboBox* QMemoryOperationListView::getThreadIdComboBox() const
{
	return m_pThreadIdComboBox;
}

QPushButton* QMemoryOperationListView::getFilterButton() const
{
	return m_pFilterButton;
}

QTreeView* QMemoryOperationListView::getTreeView() const
{
	return m_pTreeView;
}

void QMemoryOperationListView::setData(int iRow, int iCol, const QString& szValue)
{
	if(iRow == StatusBarRow_Search){
		m_listStatusRow1.value(iCol)->setText(szValue);
	}
	if(iRow == StatusBarRow_Global){
		m_listStatusRow2.value(iCol)->setText(szValue);
	}
}

QString QMemoryOperationListView::getColName(int iCol)
{
	switch(iCol)
	{
		case StatusBarCol_OpCount:
			return tr("Op count");
		case StatusBarCol_TotalAllocCount:
			return tr("Alloc total count");
		case StatusBarCol_TotalAllocSize:
			return tr("Alloc total size");
		case StatusBarCol_TotalFreeCount:
			return tr("Free total count");
		case StatusBarCol_TotalFreeSize:
			return tr("Free total size");
		case StatusBarCol_TotalRemainingCount:
			return tr("Remaining total count");
		case StatusBarCol_TotalRemainingSize:
			return tr("Remaining total size");
		case StatusBarCol_malloc:
			return tr("malloc");
		case StatusBarCol_calloc:
			return tr("calloc");
		case StatusBarCol_realloc:
			return tr("realloc");
		case StatusBarCol_free:
			return tr("free");
		case StatusBarCol_posix_memalign:
			return tr("posix_memalign");
		case StatusBarCol_aligned_alloc:
			return tr("aligned_alloc");
		case StatusBarCol_memalign:
			return tr("memalign");
		case StatusBarCol_valloc:
			return tr("valloc");
		case StatusBarCol_pvalloc:
			return tr("pvalloc");
		default:
			break;
	}

	return QString();
}