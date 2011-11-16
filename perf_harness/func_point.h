/*
 * Helper class for a single function point
 */
class FuncPoint {
  public:
    UINT32 index;
    UINT64 start_icount;
    ADDRINT start_func_addr;
    UINT64 start_func_crossings;
    UINT64 end_icount;
    ADDRINT end_func_addr;
    UINT64 end_func_crossings;

    UINT64 Length() { return end_icount - start_icount; }
    UINT64 LengthErr(UINT64 measured) { return measured - Length(); }

    friend ostream& operator<< (ostream &os, const FuncPoint &val);
    friend istream& operator>> (istream &is, FuncPoint &val);
};

ostream& operator<< (ostream &os, const FuncPoint &val)
{
    os << val.index << " ";
    os << hex << val.start_func_addr << " " 
       << dec << val.start_func_crossings << " " 
       << val.start_icount << endl;
    os << hex << val.end_func_addr << " " 
       << dec << val.end_func_crossings << " " 
       << val.end_icount << endl;
    return os;
}

istream& operator>> (istream &is, FuncPoint &val)
{
    is >> val.index >> hex >> val.start_func_addr
       >> dec >>  val.start_func_crossings >> val.start_icount;
    is >> hex >> val.end_func_addr
       >> dec >> val.end_func_crossings >> val.end_icount;
    return is;
}
