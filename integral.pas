program prog;
	type  func = function(x:real):real;
	var a,b,eps1,eps2,q1,q2,q3,x12,x23,x31:real;
	k,n1,n2,n3:integer; 
function f2(x:real):real;
 begin
  f2:=0.6*x+3;
 end;
function f3(x:real):real;
 begin
  f3:=(x-2)*(x-2)*(x-2)-1;
 end;
function f1(x:real):real;
 begin
  f1:=3/x;
 end;
 
procedure root(f,g : func; a,b,eps:real; var x:real);
	var  c:real; q:boolean;
begin
q:=true;
while q do
 begin
	c:=abs(a*(f(b)-g(b))-b*(f(a)-g(a)))/abs(f(b)-g(b)-f(a)+g(a));
	if f(a)-g(a)<0
		then
	    begin
		    if f(c)-g(c)<0 then
		      begin
			      a:=c;
			      if (f(c-eps)-g(c-eps)>0)or(f(c+eps)-g(c+eps)>0) then q:=false;
		      end
			  else
		      begin
			      b:=c;
			      if (f(c-eps)-g(c-eps)<0)or(f(c+eps)-g(c+eps)<0) then q:=false;
		      end;
	    end
	  else
	    begin
		    if f(c)-g(c)>0
		      then
		        begin
			        a:=c;
			        if (f(c-eps)-g(c-eps)<0)or(f(c+eps)-g(c+eps)<0) then q:=false;
					  end
			  else
		      begin
			      b:=c;
			      if (f(c-eps)-g(c-eps)>0)or(f(c+eps)-g(c+eps)>0) then q:=false;
		      end;
	     end;
 end;
 x:=a;
 writeln(a:0:7,' ',f(a):0:7,' с точностью ',eps);
end;

function integral(f:func;a,b,eps1:real):real;
	var s2n,fch,fnch,p,h,s:real; i,n:integer;
begin
 n:=2;
 h:=(b-a)/n;
 s:=(f(a)+f(b))*h/3;
 fnch:=f(a+1*h);
 fch:=0;
 s2n:=(f(a)+f(b)+4*fnch)*h/3;
 p:=1/15;
	while abs(p*(s-s2n))>eps1 do
	begin
		n:=n*2;
		h:=(b-a)/n;
		s:=s2n;
		fnch:=0;
		fch:=0;
		for i:=1 to n-1 do
			if i mod 2 = 1 
				then fnch:=fnch+f(a+i*h)
				else fch:=fch+f(a+i*h);
		s2n:=(s+4*fnch+2*fch)*h/3;	
	end;
 integral:=s2n;
end;


begin
a:=-1;
b:=4;
eps1:=0.001;
eps2:=0.001;
root(@f1,@f2,a,b,eps1,x12);
root(@f2,@f3,a,b,eps1,x23);
root(@f3,@f1,a,b,eps1,x31);
q1:=integral(@f1,x12,x31,eps2);
q2:=integral(@f2,x12,x23,eps2);
q3:=integral(@f3,x31,x23,eps2);
 
writeln('I1: ',q1,' с точностью ',eps2);
writeln('I2: ',q2,' с точностью ',eps2);
writeln('I3: ',q3,' с точностью ',eps2);
if (abs(q1)>abs(q2))and(abs(q1)>abs(q3))then writeln('s: ',abs(q1)-abs(q2)-abs(q3));	
if (abs(q2)>abs(q1))and(abs(q2)>abs(q3))then writeln('s: ',abs(q2)-abs(q1)-abs(q3));	
if (abs(q3)>abs(q2))and(abs(q3)>abs(q1))then writeln('s: ',abs(q3)-abs(q2)-abs(q1));	
end.
