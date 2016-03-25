
// SDCardBurnDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SDCardBurn.h"
#include "SDCardBurnDlg.h"
#include "afxdialogex.h"
#include <winioctl.h>
#include <Dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//WriteSDCardThread *pThWrite;



CFile m_CFSrcFile;
HANDLE m_HdSDCard;


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSDCardBurnDlg 对话框



CSDCardBurnDlg::CSDCardBurnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SDCARDBURN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSDCardBurnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_CBDisk);
	DDX_Control(pDX, IDC_EDIT_PATH, m_CEFilePath);
	DDX_Control(pDX, IDC_BUTTON_SELFILE, m_BuSelFile);
	DDX_Control(pDX, IDC_BUTTON_BURN, m_BuBurn);
	DDX_Control(pDX, IDC_PROGRESS, m_CProgress);
	DDX_Control(pDX, IDC_STATIC_STATE, m_CSState);
}

BEGIN_MESSAGE_MAP(CSDCardBurnDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SELFILE, &CSDCardBurnDlg::OnBnClickedButtonSelfile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BURN, &CSDCardBurnDlg::OnBnClickedButtonBurn)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSDCardBurnDlg 消息处理程序

BOOL CSDCardBurnDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

									// TODO: 在此添加额外的初始化代码

									//窗口标题
	this->SetWindowTextW(L"SD卡烧写工具V2.1");

	WCHAR	curDire[MAX_PATH];
	CString strPath;
	GetCurrentDirectory(MAX_PATH, curDire);
	strPath.Format(L"%s", curDire);
	strPath = strPath.Left(strPath.ReverseFind(L'\\'));
	strPath += L"\\ACC3.bin";
	//strPath.Format(L"%s\\ACC3.bin", curDire);
	m_CEFilePath.SetWindowTextW(strPath);
	//pThWrite = (WriteSDCardThread *)AfxBeginThread(RUNTIME_CLASS(WriteSDCardThread), 0, 1024 * 1024 * 1, CREATE_SUSPENDED);
	ScanAllDisk();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSDCardBurnDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSDCardBurnDlg::OnPaint()
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
HCURSOR CSDCardBurnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CSDCardBurnDlg::MainRun(LPVOID lParam)
{


	return 0;
}

#define RDBUFF_SIZE	(1024*1024*2)
CHAR	rdBuffer[RDBUFF_SIZE], verBuffer[RDBUFF_SIZE];

UINT CSDCardBurnDlg::WriteData(LPVOID lParam)
{
	BOOL	bRtn;
	INT		verDiff;
	DWORD	outSize;
	UINT	rdLen;
	ULONGLONG	fsSize, dealSize;
	LONG	sdPoint;
	UINT	progValue;
	CString staticStr;

	CSDCardBurnDlg *myDlg = (CSDCardBurnDlg *)lParam;
	CProgressCtrl *progress = &myDlg->m_CProgress;
	CStatic *myStatic = &myDlg->m_CSState;

	CButton *myBuFilePath = &myDlg->m_BuSelFile;
	CButton *myBuBurn = &myDlg->m_BuBurn;
	CEdit *myEditPath = &myDlg->m_CEFilePath;

	myEditPath->EnableWindow(false);
	myBuBurn->EnableWindow(false);
	myBuFilePath->EnableWindow(false);

	//删除磁盘引导信息(扇区0有一个偏移信息，如果不清除，会导致写入的地址与想要的不一样)
	bRtn = DeviceIoControl(m_HdSDCard, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"删除引导信息失败:");
		goto __LAB_ERROR;
	}
	//清除扇区0
	memset(rdBuffer, 0, 512);
	SetFilePointer(m_HdSDCard, 0, 0, FILE_BEGIN);
	bRtn = WriteFile(m_HdSDCard, rdBuffer, 512, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"清除扇区0失败:");
		goto __LAB_ERROR;
	}
	//设置SD卡的偏移点
	sdPoint = 1024 * 1024 * 1;
	//源文件长度
	fsSize = m_CFSrcFile.GetLength();
	dealSize = 0;
	//开始写文件
	while (dealSize < fsSize)
	{
		rdLen = m_CFSrcFile.Read(rdBuffer, RDBUFF_SIZE);
		if (rdLen)
		{
			//写入SD卡
			SetFilePointer(m_HdSDCard, sdPoint, NULL, FILE_BEGIN);
			bRtn = WriteFile(m_HdSDCard, rdBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"写入SD卡失败:");
				goto __LAB_ERROR;
			}
			//从SD卡读出
			SetFilePointer(m_HdSDCard, sdPoint, NULL, FILE_BEGIN);
			bRtn = ReadFile(m_HdSDCard, verBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"从SD卡读出数据失败:");
				goto __LAB_ERROR;
			}
			//校验
			verDiff = memcmp(rdBuffer, verBuffer, rdLen);
			if (verDiff != 0)
			{
				AfxMessageBox(L"校验数据出错", MB_OK | MB_ICONERROR);
				goto __LAB_ERROR;
			}
			sdPoint += rdLen;
			dealSize += rdLen;

			progValue = (INT)(((float)dealSize / (float)fsSize) * 100);
			progress->SetPos(progValue);
			staticStr.Format(L"%u%%", progValue);
			myStatic->SetWindowTextW(staticStr);
		}
		//Sleep(10);
	}

	progress->SetPos(100);
	AfxMessageBox(L"写数据成功", MB_OK | MB_ICONMASK);

__LAB_ERROR:
	myEditPath->EnableWindow(true);
	myBuBurn->EnableWindow(true);
	myBuFilePath->EnableWindow(true);
	CloseHandle(m_HdSDCard);
	m_CFSrcFile.Close();
	AfxEndThread(0);
	return 0;
}



void CSDCardBurnDlg::OnBnClickedButtonSelfile()
{
	// TODO: 在此添加控件通知处理程序代码

	CString filePath;
	BOOL bRes;
	WCHAR	curDire[MAX_PATH];
	CFileDialog dlgFile(true, NULL, L"", OFN_HIDEREADONLY, L"bin文件(*.bin)|*.bin|", NULL);
	OPENFILENAME& ofn = dlgFile.GetOFN();
	GetCurrentDirectory(MAX_PATH, curDire);

	ofn.lpstrInitialDir = curDire;
	dlgFile.ApplyOFNToShellDialog();
	if (dlgFile.DoModal() == IDOK)
	{
		filePath = dlgFile.GetPathName();
		m_CEFilePath.SetWindowTextW(filePath);
	}
	else
	{
		//MessageBox(L"选择文件错误", L"错误", MB_OK | MB_ICONERROR);
	}
}


void CSDCardBurnDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}


void CSDCardBurnDlg::OnBnClickedButtonBurn()
{
	// TODO: 在此添加控件通知处理程序代码
	WCHAR textBuff[1024];
	CString str;
	BOOL bRtn;

	//获取源文件路径
	m_CEFilePath.GetWindowTextW(textBuff, m_CEFilePath.GetWindowTextLengthW() + 1);
	str = textBuff;
	if (str.IsEmpty())
	{
		MessageBox(L"路径不能为空", L"错误", MB_OK | MB_ICONERROR);
		return;
	}
	//打开源文件
	bRtn = m_CFSrcFile.Open(str, CFile::modeRead | CFile::typeBinary | CFile::shareDenyRead);
	if (!bRtn)
	{
		MessageBox(L"打开源文件失败", L"错误", MB_OK | MB_ICONERROR);
		return;
	}

	//创建磁盘句柄
	//m_CBDisk.GetWindowTextW(str);
	if (m_CBDisk.GetCount() == 0)
	{
		m_CFSrcFile.Close();
		MessageBox(L"请选择磁盘", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}
	m_CBDisk.GetLBText(m_CBDisk.GetCurSel(), str);
	if (str.IsEmpty())
	{
		MessageBox(L"请选择磁盘", L"警告", MB_OK | MB_ICONWARNING);
		m_CFSrcFile.Close();
		return;
	}
	str = L"\\\\.\\" + str;
	m_HdSDCard = CreateFile(str, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (m_HdSDCard == INVALID_HANDLE_VALUE)
	{
		m_CFSrcFile.Close();
		AfxMessageBox(L"打开SD卡出错", MB_OK | MB_ICONERROR);
		return;
	}

	//m_TimerID = SetTimer(0, 10, 0);
	m_CSState.SetWindowTextW(L"0%");
	m_CProgress.SetPos(0);
	m_TFunWriteData = AfxBeginThread((AFX_THREADPROC)WriteData, this, THREAD_PRIORITY_NORMAL, 1024 * 1024 * 1, CREATE_SUSPENDED, NULL);
	m_TFunWriteData->ResumeThread();
}


void CSDCardBurnDlg::ScanAllDisk(void)
{
	TCHAR szBuf[100];
	DWORD len, i;
	UINT type;
	CString str;
	memset(szBuf, 0, 100);

	len = GetLogicalDriveStrings(sizeof(szBuf) / sizeof(TCHAR), szBuf);
	m_CBDisk.ResetContent();

	for (i = 0; i<len; i++)
	{
		str = &szBuf[i];
		type = GetDriveType(str);
		if (type == DRIVE_REMOVABLE)
		{
			str.Delete(2, 1);
			m_CBDisk.AddString(str);
		}
	}
	i = m_CBDisk.GetCount();
	if (i > 0)
	{
		m_CBDisk.SetCurSel(i - 1);
	}
	else
	{
		m_CBDisk.SetCurSel(0);
	}
}

void CSDCardBurnDlg::ViewLastError(LPTSTR lpMsg)
{

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();//获取错误代码

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	CString strErr;
	strErr.Format(L"%s%s", lpMsg, lpMsgBuf);
	MessageBox(strErr, L"错误", MB_OK | MB_ICONERROR);
	LocalFree(lpMsgBuf);
}




LRESULT CSDCardBurnDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//WM_DEVICECHANGE，系统硬件改变发出的系统消息
	case WM_DEVICECHANGE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		switch (wParam)
		{
		case WM_DEVICECHANGE:
			break;
		case DBT_DEVICEARRIVAL://DBT_DEVICEARRIVAL，设备检测结束，并且可以使用
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//逻辑卷
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U盘
				{
					ScanAllDisk();
				}
				break;
				case DBTF_MEDIA:    //光盘
					break;
				}
			}
		}
		break;
		case DBT_DEVICEREMOVECOMPLETE://DBT_DEVICEREMOVECOMPLETE,设备卸载或者拔出
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//逻辑卷
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U盘
				{
					ScanAllDisk();
				}
				break;
				case DBTF_MEDIA:    //光盘

					break;
				}
			}
		}
		break;
		}
	}
	break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}


void CSDCardBurnDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	CDialogEx::OnTimer(nIDEvent);
}
