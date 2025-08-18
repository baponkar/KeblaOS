#include "terminal.h"

uint64_t term_width = 0;
uint64_t term_height = 0;

uint64_t cur_pos_x = 0;
uint64_t cur_pos_y = 0;

uint32_t text_col = 0;
uint32_t back_col = 0;

uint64_t font_siz = 0;



void init_term(){
    term_width = 0;
    term_height = 0;

    text_col = 0;
    back_col = 0;

    font_siz = 0;

    cur_pos_x = 0;
    cur_pos_y = 0;

    
}






