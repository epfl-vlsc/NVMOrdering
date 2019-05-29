void a();
void b();
void c();
void d();


void pathexplode(int x, int y){
    if(x){
        if(y){
            a();
        }else{
            b();
        }
    }else{
        if(y){
            c();
        }else{
            d();
        }
    }
}

