#ifndef _MDO_Tools_H
#define _MDO_Tools_H


namespace MDO {

/**
 * 
 */ 
class Tools {
	
	private:
	
	private:
	
	public:
		static unsigned long	millisDiff(const unsigned long& ulStart, const unsigned long& ulEnd);
		static unsigned long	millisDiff(const unsigned long& ulStart);

	private:
		Tools();
	public:
		virtual ~Tools();
};

}	//namespace end

#endif