#include <stdio.h>

int main(){
int val;
char phone[11];
int error = 0;
scanf("%s\n", phone);
while(scanf("%d", &val) != EOF) {
if(val == -1){
printf("%s\n",phone);
}
else if(val >=0 && val <= 9){
printf("%c\n",phone[val]);
}
else{
printf("ERROR\n");
error = 1;
}
}
return error;
}
