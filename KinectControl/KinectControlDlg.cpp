
// KinectControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "KinectControl.h"
#include "KinectControlDlg.h"
#include "afxdialogex.h"
#include <afxpriv.h> // 主要使用WM_KICKIDLE消息用到

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CKinectControlDlg 对话框




CKinectControlDlg::CKinectControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKinectControlDlg::IDD, pParent), 
	  m_dlgSkel(this),
	  m_dlgSetAngle(this),
	  m_bControl(false),
	  /*m_bSetOrigin(false),
	  m_bOriginSet(false),*/
	  m_bShowSkeleton(false),
	  m_bSensorReady(false),
	  m_bMeasureBody(false),
	  m_bBodyMeasured(false),
	  m_bDragging(false),
	  m_bWaitForDrag(false),
	  m_bNoMove(false),
	  m_bReverseHand(false),
	  m_bLeftDown(false),
	  m_bMoveLocked(false),
	  m_EvNextSkeletonEvent(FALSE, TRUE, NULL, NULL),
	  m_EvNextDepthFrameEvent(FALSE, TRUE, NULL, NULL),
	  m_TrackingMode(SV_TRACKED_SKELETONS_DEFAULT)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_SystemRes.cx = GetSystemMetrics(SM_CXSCREEN);
	m_SystemRes.cy = GetSystemMetrics(SM_CYSCREEN);
	m_hWaitForDragCur = LoadCursorFromFile(_T("res\\Kinect Cursor(beta).ani"));
}

CKinectControlDlg::~CKinectControlDlg()
{
	//Nui_UnInit();
}

void CKinectControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRACKMODE_LIST, m_CmbTrackMode);
}

BEGIN_MESSAGE_MAP(CKinectControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_KICKIDLE,OnKickIdle)
	ON_BN_CLICKED(IDC_BTN_MSRBDY, &CKinectControlDlg::OnClickedBtnMsrbdy)
	ON_BN_CLICKED(IDC_BTN_SHOWSKEL, &CKinectControlDlg::OnClickedBtnShowskel)
	ON_BN_CLICKED(IDC_BTN_STARTCTRL, &CKinectControlDlg::OnClickedBtnStartctrl)
	ON_WM_CLOSE()
//	ON_BN_CLICKED(IDC_BTN_CHANGE_TRACKING_MODE, &CKinectControlDlg::OnClickedBtnChangeTrackingMode)
	ON_BN_CLICKED(IDC_BTN_ACTBANDING, &CKinectControlDlg::OnClickedBtnActbanding)
//	ON_WM_SETCURSOR()
	ON_CBN_SELCHANGE(IDC_TRACKMODE_LIST, &CKinectControlDlg::OnSelchangeTrackmodeList)
	ON_BN_CLICKED(IDC_BTN_SETTINGS, &CKinectControlDlg::OnBnClickedBtnSettings)
END_MESSAGE_MAP()


// CKinectControlDlg 消息处理程序

BOOL CKinectControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	if(!m_dlgSkel.Create(IDD_SKELETALVIEWER, this))
		AfxMessageBox(_T("初始化对话框失败！"));
	if(!m_dlgSetAngle.Create(IDD_SET_KINECT_ANGLE, this))
		AfxMessageBox(_T("初始化对话框失败！"));

	Nui_Zero();
	if(SUCCEEDED(Nui_Init()))
	{
		GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Ready"));
		m_bSensorReady = true;
	}
	else
	{
		GetDlgItem(IDC_STATIC_STATE)->SetWindowText(_T("Not Ready"));
		m_bSensorReady = false;
	}

	m_CmbTrackMode.SetCurSel(m_TrackingMode);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CKinectControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CKinectControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CKinectControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 利用WM_KICKIDLE消息构建UI更新机制
LRESULT CKinectControlDlg::OnKickIdle(WPARAM wParam, LPARAM lParam) 
{ 
	//UpdateDialogControls(this, TRUE);
	GetDlgItem(IDC_BTN_MSRBDY)->EnableWindow(m_bSensorReady);
	//GetDlgItem(IDC_BTN_SETORIGIN)->EnableWindow(m_bSensorReady);
	GetDlgItem(IDC_BTN_SHOWSKEL)->EnableWindow(m_bSensorReady);
	//GetDlgItem(IDC_BTN_CHANGE_TRACKING_MODE)->EnableWindow(m_bSensorReady);
	GetDlgItem(IDC_BTN_STARTCTRL)->EnableWindow(m_bSensorReady && m_bBodyMeasured);
	if(m_bControlStateChanged)
	{
		SetDlgItemTextW(IDC_BTN_STARTCTRL, m_bControl? _T("结束控制"): _T("开始控制"));
		m_bControlStateChanged = false;
	}
	//if(m_bTrackingModeChanged)
	//{
	//	static TCHAR szRes[512];
	//	LoadString(AfxGetInstanceHandle(), IDS_TRACKMODE_DEFAULT + m_TrackingMode, szRes, _countof(szRes) );
	//	SetDlgItemText(IDC_STATIC_TRACKING_MODE, szRes);
	//	m_bTrackingModeChanged = false;
	//}
	return 0;
}

/// <summary>
/// Display a MessageBox with a string table table loaded string
/// </summary>
/// <param name="nID">id of string resource</param>
/// <param name="nType">type of message box</param>
/// <returns>result of MessageBox call</returns>
int CKinectControlDlg::MessageBoxResource( UINT nID )
{
    static TCHAR szRes[512];

    LoadString( AfxGetInstanceHandle(), nID, szRes, _countof(szRes) );
    return MessageBox(szRes);
}

void CKinectControlDlg::SetBodyInfo()
{
	CString str;
	str.Format(_T("%.1f"),m_shoulderWidth * 100);
	TRACE("%.1f\n",m_shoulderWidth * 100);
	GetDlgItem(IDC_BODY_HEIGHT)->SetWindowTextW(LPCWSTR(str));
	str.Format(_T("%.1f"),m_armLength * 100);
	TRACE("%.1f\n",m_armLength * 100);
	GetDlgItem(IDC_ARM_LENGTH)->SetWindowTextW(LPCWSTR(str));
	str.Format(_T("%.1f"),m_spineLength * 100);
	TRACE("%.1f\n",m_spineLength * 100);
	GetDlgItem(IDC_SPINE_LENGTH)->SetWindowTextW(LPCWSTR(str));
}

void CKinectControlDlg::OnClickedBtnMsrbdy()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bMeasureBody = true;
}


//void CKinectControlDlg::OnClickedBtnSetorigin()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	m_dlgSkel.SetWithGuide(true);
//	m_dlgSkel.ShowWindow(SW_SHOW);
//	m_bSetOrigin = true;
//	m_bOriginSet = true;
//}


void CKinectControlDlg::OnClickedBtnShowskel()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bShowSkeleton = true;
	m_dlgSkel.SetWithGuide(false);
	m_dlgSkel.ShowWindow(SW_SHOW);
}


void CKinectControlDlg::OnClickedBtnStartctrl()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bControl = !m_bControl;
	m_bControlStateChanged = true;
}


void CKinectControlDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bControl)
		m_bControl = false;
	if(m_bShowSkeleton)
		m_bShowSkeleton = false;
	Nui_UnInit();
	CDialogEx::OnClose();
}


//void CKinectControlDlg::OnClickedBtnChangeTrackingMode()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	m_TrackingMode = m_TrackingMode == SV_TRACKED_SKELETONS_STICKY?
//										SV_TRACKED_SKELETONS_NEAREST:
//										SV_TRACKED_SKELETONS_STICKY;
//
//}


void CKinectControlDlg::OnClickedBtnActbanding()
{
	// TODO: 在此添加控件通知处理程序代码
	//m_bWaitForDrag = !m_bWaitForDrag;
}


//BOOL CKinectControlDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	/*if(!m_bWaitForDrag)
//	{
//		SetCursor(m_hWaitForDragCur);
//		return true;
//	}*/
//	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
//}


void CKinectControlDlg::OnSelchangeTrackmodeList()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_CmbTrackMode.GetCurSel();
	UpdateTrackedSkeletonSelection(index);

	//m_bTrackingModeChanged = true;
}


void CKinectControlDlg::OnBnClickedBtnSettings()
{
	// TODO: 在此添加控件通知处理程序代码
	m_dlgSetAngle.ShowWindow(SW_SHOW);
	m_bShoeSetAngle = true;
}

