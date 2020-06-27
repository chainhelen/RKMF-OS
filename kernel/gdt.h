#ifndef GDT_H
#define GDT_H


struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

void load_tr(int tr);
void farjmp(int eip, int cs);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);


#endif 
