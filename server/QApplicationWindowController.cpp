//
// Created by ebeuque on 11/03/2021.
//

#include <QTreeView>
#include <QHeaderView>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QElapsedTimer>

#include "QApplicationWindow.h"
#include "QMemoryOperationModel.h"

#include "QApplicationWindowController.h"

QApplicationWindowController::QApplicationWindowController()
{
	m_pApplicationWindow = NULL;
	m_pModels = NULL;
}

QApplicationWindowController::~QApplicationWindowController()
{

}

bool QApplicationWindowController::init(QApplicationWindow* pApplicationWindow)
{
	m_pApplicationWindow = pApplicationWindow;

	m_pModels = new QMemoryOperationModel();
	m_pModels->setMemoryOperationList(&m_listFilterMemoryOperation);

	QTreeView* pTreeView = m_pApplicationWindow->getTreeView();
	pTreeView->setModel(m_pModels);
	//pTreeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	pTreeView->header()->resizeSection(0, 200);
	pTreeView->header()->resizeSection(1, 150);
	pTreeView->header()->resizeSection(2, 100);
	pTreeView->header()->resizeSection(3, 100);
	pTreeView->header()->resizeSection(4, 150);
	pTreeView->header()->resizeSection(5, 100);
	pTreeView->header()->resizeSection(6, 150);

	connect(m_pApplicationWindow->getSearchButton(), SIGNAL(clicked()), this, SLOT(onSearchButtonClicked()));

	m_timerUpdate.setInterval(1000);
	connect(&m_timerUpdate, SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
	m_timerUpdate.start();

	return true;
}

void QApplicationWindowController::addMemoryOperation(const QSharedPointer<MemoryOperation>& pMemoryOperation)
{
	MemoryOperationSharedPtr pMemoryOperationFreed;

	m_lockListMemoryOperation.lockForWrite();
	if(pMemoryOperation->m_iFreePtr){
		pMemoryOperationFreed = m_listMemoryOperation.getPtrNotFreed(pMemoryOperation->m_iFreePtr);
		if(pMemoryOperationFreed) {
			pMemoryOperation->m_bFreed = true;
		}
	}
	m_listMemoryOperation.append(pMemoryOperation);

	m_lockGlobalStats.lockForWrite();
	m_globalStats.m_iOpCount++;
	m_globalStats.m_iAllocSize += pMemoryOperation->m_iAllocSize;
	m_globalStats.m_iRemaingSize += pMemoryOperation->m_iAllocSize;
	if(pMemoryOperationFreed){
		m_globalStats.m_iFreeSize += pMemoryOperationFreed->m_iAllocSize;
		m_globalStats.m_iRemaingSize -= pMemoryOperationFreed->m_iAllocSize;
	}
	switch (pMemoryOperation->m_iMemOpType) {
	case ALeakD_malloc:
		m_globalStats.m_iMallocCount++; break;
	case ALeakD_calloc:
		m_globalStats.m_iCallocCount++; break;
	case ALeakD_realloc:
		m_globalStats.m_iReallocCount++; break;
	case ALeakD_free:
		m_globalStats.m_iFreeCount++; break;
	case ALeakD_posix_memalign:
		m_globalStats.m_iPosixMemalignCount++; break;
	case ALeakD_aligned_alloc:
		m_globalStats.m_iAlignedAllocCount++; break;
	case ALeakD_memalign:
		m_globalStats.m_iMemAlignCount++; break;
	case ALeakD_valloc:
		m_globalStats.m_iVAllocCount++; break;
	case ALeakD_pvalloc:
		m_globalStats.m_iPVAllocCount++; break;
	}
	m_lockGlobalStats.unlock();

	m_lockListMemoryOperation.unlock();
}

void QApplicationWindowController::clearMemoryOperation()
{
	m_lockListMemoryOperation.lockForWrite();
	m_listMemoryOperation.clear();
	m_lockListMemoryOperation.unlock();

	m_searchStats.reset();
	m_lockGlobalStats.lockForWrite();
	m_globalStats.reset();
	m_lockGlobalStats.unlock();

	m_listFilterMemoryOperation.clear();
	m_pModels->clear();
}

void QApplicationWindowController::onSearchButtonClicked()
{
	bool bNotFreed = true;

	QElapsedTimer timer;
	qDebug("[aleakd-server] Start search");
	timer.start();

	m_searchStats.reset();

	m_listFilterMemoryOperation.clear();
	m_lockListMemoryOperation.lockForRead();
	MemoryOperationList::const_iterator iter;
	for(iter = m_listMemoryOperation.constBegin(); iter != m_listMemoryOperation.constEnd(); ++iter)
	{
		bool bAccept = true;
		MemoryOperationSharedPtr pMemoryOperation = (*iter);
		if(bNotFreed){
			if(pMemoryOperation->m_iMemOpType == ALeakD_free){
				bAccept = false;
			}
			if(pMemoryOperation->m_bFreed){
				bAccept = false;
			}
		}
		if(bAccept) {
			m_searchStats.m_iOpCount++;
			m_searchStats.m_iAllocSize += pMemoryOperation->m_iAllocSize;
			m_searchStats.m_iRemaingSize += pMemoryOperation->m_iAllocSize;
			if(pMemoryOperation->m_bFreed){
				m_searchStats.m_iFreeSize += pMemoryOperation->m_iAllocSize;
				m_searchStats.m_iRemaingSize -= pMemoryOperation->m_iAllocSize;
			}
			switch (pMemoryOperation->m_iMemOpType) {
			case ALeakD_malloc:
				m_searchStats.m_iMallocCount++; break;
			case ALeakD_calloc:
				m_searchStats.m_iCallocCount++; break;
			case ALeakD_realloc:
				m_searchStats.m_iReallocCount++; break;
			case ALeakD_free:
				m_searchStats.m_iFreeCount++; break;
			case ALeakD_posix_memalign:
				m_searchStats.m_iPosixMemalignCount++; break;
			case ALeakD_aligned_alloc:
				m_searchStats.m_iAlignedAllocCount++; break;
			case ALeakD_memalign:
				m_searchStats.m_iMemAlignCount++; break;
			case ALeakD_valloc:
				m_searchStats.m_iVAllocCount++; break;
			case ALeakD_pvalloc:
				m_searchStats.m_iPVAllocCount++; break;
			}
			m_listFilterMemoryOperation.append(*iter);
		}
	}
	m_lockListMemoryOperation.unlock();
	m_pModels->clear();
	m_pModels->fetchTo(m_listFilterMemoryOperation.count());

	qDebug("[aleakd-server] Search done in %ld ms", timer.elapsed());
}

void QApplicationWindowController::onTimerUpdate()
{
	m_lockGlobalStats.lockForRead();

	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_OpCount, QString::number(m_searchStats.m_iOpCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_AllocSize, QString::number(m_searchStats.m_iAllocSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_FreeSize, QString::number(m_searchStats.m_iFreeSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_RemainingSize, QString::number(m_searchStats.m_iRemaingSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_malloc, QString::number(m_searchStats.m_iMallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_calloc, QString::number(m_searchStats.m_iCallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_realloc, QString::number(m_searchStats.m_iReallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_free, QString::number(m_searchStats.m_iFreeCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_posix_memalign, QString::number(m_searchStats.m_iPosixMemalignCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_aligned_alloc, QString::number(m_searchStats.m_iAlignedAllocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_memalign, QString::number(m_searchStats.m_iMemAlignCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_valloc, QString::number(m_searchStats.m_iVAllocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Search, QApplicationWindow::StatusBarCol_pvalloc, QString::number(m_searchStats.m_iPVAllocCount));

	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_OpCount, QString::number(m_globalStats.m_iOpCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_AllocSize, QString::number(m_globalStats.m_iAllocSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_FreeSize, QString::number(m_globalStats.m_iFreeSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_RemainingSize, QString::number(m_globalStats.m_iRemaingSize));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_malloc, QString::number(m_globalStats.m_iMallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_calloc, QString::number(m_globalStats.m_iCallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_realloc, QString::number(m_globalStats.m_iReallocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_free, QString::number(m_globalStats.m_iFreeCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_posix_memalign, QString::number(m_globalStats.m_iPosixMemalignCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_aligned_alloc, QString::number(m_globalStats.m_iAlignedAllocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_memalign, QString::number(m_globalStats.m_iMemAlignCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_valloc, QString::number(m_globalStats.m_iVAllocCount));
	m_pApplicationWindow->setData(QApplicationWindow::StatusBarRow_Global, QApplicationWindow::StatusBarCol_pvalloc, QString::number(m_globalStats.m_iPVAllocCount));

	m_lockGlobalStats.unlock();
}

void QApplicationWindowController::onMemoryOperationReceived(const MemoryOperationSharedPtr& pMemoryOperation)
{
	addMemoryOperation(pMemoryOperation);
}

void QApplicationWindowController::onNewConnection()
{
	clearMemoryOperation();
}