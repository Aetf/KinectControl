// SkeletalViewerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "resource.h"
#include "KinectControl.h"
#include "SkeletalViewerDlg.h"
#include "afxdialogex.h"


// CSkeletalViewerDlg 对话框

IMPLEMENT_DYNAMIC(CSkeletalViewerDlg, CDialogEx)

CSkeletalViewerDlg::CSkeletalViewerDlg(CWnd* pParent)
	: CDialogEx(CSkeletalViewerDlg::IDD, pParent),
	m_LastSkeletonFoundTime(0)
{
	m_LastFPStime = timeGetTime();
}

CSkeletalViewerDlg::~CSkeletalViewerDlg()
{
	Nui_UnInit();
}

void CSkeletalViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_STATIC_GUIDE, m_imgGuide);
}


BEGIN_MESSAGE_MAP(CSkeletalViewerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CLOSE, &CSkeletalViewerDlg::OnClickedClose)
END_MESSAGE_MAP()


// CSkeletalViewerDlg 消息处理程序


BOOL CSkeletalViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//m_imgGuide.ModifyStyle(0xF, SS_BITMAP|SS_CENTERIMAGE);
	//m_imgGuide.SetBitmap(::LoadBitmap(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP1)));
	Nui_Zero();
	Nui_Init();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSkeletalViewerDlg::SetWithGuide(bool withGuide)
{
	m_bWithGuide = withGuide;
	//m_imgGuide.ShowWindow(withGuide);
	GetDlgItem(IDC_CLOSE)->SetWindowText(withGuide? _T("设定") : _T("关闭"));
}

void CSkeletalViewerDlg::Nui_RestartTimer()
{
	m_bScreenBlanked = false;
	m_LastSkeletonFoundTime = timeGetTime();
	m_FramesTotal++;  // 每调用一次此函数说明产生了新的一帧骨骼图
}

void CSkeletalViewerDlg::SetFPS()
{
	RECT rct;
	int t;

	// Once per second, display the FPS
	t = timeGetTime( );
	if ( (t - m_LastFPStime) > 1000 )
	{
		int fps = ((m_FramesTotal - m_LastFramesTotal) * 1000 + 500) / (t - m_LastFPStime);
		m_LastFramesTotal = m_FramesTotal;
		m_LastFPStime = t;
		SetDlgItemInt(IDC_FPS, fps, FALSE);
		GetDlgItem(IDC_FPS)->GetWindowRect(&rct);
		ValidateRect(&rct);
	}
}


void CSkeletalViewerDlg::OnClickedClose()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ShowWindow(SW_HIDE);
}
