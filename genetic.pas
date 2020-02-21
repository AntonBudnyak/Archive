program generate;
const v1=0.01;{вероятность на скрещивание при коэф <0}
      v2=0.5;{вероятность выбрать ген 1-го родителя}
      v3=1/16;{вероятность гена мутировать}
      k=100;{проходов до остоновы}
      l=5;{кол-во цифр после запятой в печати}
type
  chances = array[1..16]of real;
  number = array[1..16]of integer;
  nabor = array[1..16]of number;

var
  i,j: integer;pop: nabor;ch: chances;

function f(x: real): real;
begin
  f := x * sin(x + 5) * cos(x - 6) * sin(x + 7) * cos(x - 8) * sin(x / 3 + 0.2);
end;

function step(x, n: integer): real;
var
  i, s: integer;
begin
  if n = 0 then step := 1
  else if n > 0 then
  begin
    s := 1;
    for i := 1 to n do
      s := s * x;
    step := s;
  end
  else step := 1 / step(x, -n);
end;


procedure tobin(x: real; var a: number);
var
  i: integer;
begin
  case trunc(x) of
    1: begin a[1] := 0; a[2] := 1; end;
    2: begin a[1] := 1; a[2] := 0; end;
    3: begin a[1] := 1; a[2] := 1; end;
  end;
  x := x - trunc(x);
  for i := 3 to 16 do
  begin
    a[i] := trunc(2 * x);
    x := 2 * x - trunc(2 * x);      
  end;      
end;

function todec(a: number): real;
var
  i: integer;x: real;
begin
  x := 0;
  for i := 1 to 16 do
    x := x + step(2, -i + 2) * a[i];
  todec := x;
end;

procedure selection(pop: nabor; var ch: chances);
var
  i,m: integer;k,s: real;
begin
  s := 0;
  m:=1;
  for i := 1 to 16 do
  begin
    ch[i] := f(todec(pop[i]));
    if ch[i]<ch[m] then m:=i;
    s := s + ch[i];
  end;
  k:=ch[m];
  {writeln('m=',m,f(todec(pop[m])):3:3);}
  s := s/16 - ch[m];
  {writeln('s=',s:3:3);}
  for i := 1 to 16 do
    ch[i] := (ch[i]-k)/s;
  ch[m]:=v1;
end;

function rand(x: real): boolean;
begin
  if x=0 then rand:=false else rand := random(maxint)/maxint < x;
end;


procedure cross(var pop: nabor;var ch: chances);
var
  i,j,d1,d2 :integer; res: nabor;
begin
  for i := 1 to 16 do
  begin
    d1 := 0;
    d2 := 0;
    while (d1 = 0) or (d2 = 0) do
    begin
      j := 1+ random(16);
      if (ch[j] > 1) or (rand(ch[j])) then
      begin
        if ch[j]>1 then ch[j] := ch[j]-0.25 else ch[j]:=ch[j]/5;
        if d1 = 0 then d1 := j else d2 := j;
      end;
    end; 
    for j:=1 to 16 do
      if rand(v2) then res[i,j]:=pop[d1,j] else res[i,j]:=pop[d2,j];
  end;
  pop:=res;
end;

procedure mutation(var pop:nabor);
var i,j:integer;
begin
for i:=1 to 16 do
  for j:= 1 to 16 do
    if rand(v3) then pop[i,j]:=abs(pop[i,j]-1);
end;

begin
  randomize;
  for i := 1 to 16 do
    tobin(4*random(maxint)/maxint, pop[i]);
  for i:=1 to k do 
    begin
    {for j:=1 to 16 do 
              write(todec(pop[j]):3:3,' ');
      writeln;
      for j:=1 to 16 do 
              write(f(todec(pop[j])):3:3,' ');
              writeln;}
      selection(pop, ch);
      {for j:=1 to 16 do 
      write(ch[j]:3:3,' ');
      writeln;}

      cross(pop,ch);
      mutation(pop);
      { writeln;}
    end;
i:=1;
for j := 2 to 16 do
      if f(todec(pop[j]))>f(todec(pop[i])) then i:=j;
writeln('x=',todec(pop[i]):3:3,' y=',f(todec(pop[i])):3:3);
end.
