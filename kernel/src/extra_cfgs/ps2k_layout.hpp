#ifndef PS2K_LAYOUT_HPP
#define PS2K_LAYOUT_HPP 1

// in #define macros because i don't trust gcc, also to help swap the layout whenever you want
// for example you can just comment the layout you dont like and create your own and its the quickest way

const char us_default_layout_norm[128] = {
    0, '`','1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,
    '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' ',0
};

const char us_default_layout_shft[128] = {
    0, '~','!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',0,
    '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,' ',0
};

const char us_default_layout_caps[128] = {
    0, '`','1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','[',']','\n',
    0,'A','S','D','F','G','H','J','K','L',';','\'','`',0,
    '\\','Z','X','C','V','B','N','M',',','.','/',0,
    '*',0,' ',0
};

#define translate_n us_default_layout_norm
#define translate_c us_default_layout_caps
#define translate_s us_default_layout_shft

#endif
