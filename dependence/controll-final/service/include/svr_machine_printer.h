#ifndef _SVR_MACHINE_PRINTER_H_
#define _SVR_MACHINE_PRINTER_H_

typedef struct _PrinterAttr{
    char model[128];
    char is_pluging;
    char is_have_media;
    char is_busy;
    int  media_address;
    int  tri_color_ink_level;
    int  black_color_ink_level;
    int  status;
    int  error_state;	
    int  status_code;
    char status_desc[1024];
}PrinterAttr;

class SvrPrinter
{
private:
    PrinterAttr     m_printer = {0};
private:
    SvrPrinter(){}
    ~SvrPrinter(){}

public:
	static SvrPrinter& Inst()
	{
		static SvrPrinter inst;
		return inst;
	}
    
    inline PrinterAttr* lp(){return &m_printer;}

    int GetPrintStatus();

    int GetPrintStatusEx(const char* doc_name);
};

#endif //!_SVR_MACHINE_PRINTER_H_