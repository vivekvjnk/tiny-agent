// simple palindrome check

/*
Logic: reverse the order of characters in the string
*/
#include <stdio.h>
#include <string.h>

void invert_string(char *s, int length){
    for(int i=0;i<length/2;i++){
        char tmp= s[i];
        s[i] = s[length-1-i];
        s[length-1-i] = tmp; 
    }
}

int string_compare(const char *s1, const char *s2, int len_s1, int len_s2){
    if(len_s1 != len_s2)
        return 0;
    
    for(int i=0; i< len_s1; i++){
        if(s1[i] != s2[i])
        {
            return 0;
        }
    }
    return 1;
}

int check_palindrome(const char *s, int length){
    char inv_s[length];
    memcpy(inv_s,s,length);
    invert_string(inv_s,length);
    return string_compare(inv_s,s,length,length);
}

int check_palindrome_wo_copy(const char *s, int length){
    if (length <= 0) return 1;
    for(int i=0;i<=length/2;i++){
        if(s[i] != s[length-1-i])
            return 0;
    }
    return 1;
}

int main(){
    const char str[]="alaalaal";
    printf("string is palindrome = %d\n",check_palindrome_wo_copy(str, sizeof(str)-1));
    printf("palindrome check with copy = %d\n",check_palindrome(str,sizeof(str)-1));
}