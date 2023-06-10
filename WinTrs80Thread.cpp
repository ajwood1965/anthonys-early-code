// WinTrs80Thread.cpp : implementation file
//

#include "stdafx.h"
#include "WinTrs80Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinTrs80Thread

//IMPLEMENT_DYNCREATE(CWinTrs80Thread, CWinThread)
IMPLEMENT_DYNAMIC(CWinTrs80Thread, CWinThread)

BEGIN_MESSAGE_MAP(CWinTrs80Thread, CWinThread)
	//{{AFX_MSG_MAP(CWinTrs80Thread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CWinTrs80Thread::CWinTrs80Thread(CWnd *client_wnd, CWinTrs80Thread** ptr, CTrs80Configurator *config)
{
	m_client_hwnd = client_wnd->m_hWnd;
	m_ptr_to_me = ptr;
	m_config = config;
}

CWinTrs80Thread::~CWinTrs80Thread()
{
	*m_ptr_to_me = NULL;
}

//**************************************************

BOOL CWinTrs80Thread::InitInstance()
{

	SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);

	m_cdc.Attach(::GetDC(m_client_hwnd));
	m_trs80.AttachCDC(&m_cdc);

	m_trs80.Boot(m_config);

	m_trs80.DetachCDC();
	::ReleaseDC(m_client_hwnd, m_cdc.m_hDC);
	m_cdc.Detach();

	// avoid entering standard message loop by returning FALSE
	return FALSE;
}

//**************************************************

int CWinTrs80Thread::ExitInstance()
{

	return CWinThread::ExitInstance();
}

//**************************************************

void CWinTrs80Thread::Paint(CDC* cdc)
{
	if (m_cdc.m_hDC)
		m_trs80.Paint(cdc);
}

//**************************************************

void CWinTrs80Thread::SetCharDown(unsigned char c, unsigned char oem)
{
		m_trs80.SetCharDown(c, oem);
}

//**************************************************

void CWinTrs80Thread::SetCharUp(unsigned char c, unsigned char oem)
{
	m_trs80.SetCharUp(c, oem);
}


//**************************************************

void CWinTrs80Thread::KillThreadAsync()
{
	m_trs80.SuspendExecutionAsync();
}

//**************************************************

void CWinTrs80Thread::GetDisplaySize(CDC *dc, CSize *size)
{
	m_trs80.GetDisplaySize(dc, size);
}

/////////////////////////////////////////////////////////////////////////////
// CWinTrs80Thread message handlers


