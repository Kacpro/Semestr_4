-module(onp).

-import(math,[sin/1, cos/1, tan/1, ctg/1, pow/2, sqrt/1, pi/0]).

-export([onp/1]).



onpHelper([],[H|_]) when (H < 1.0e-15) -> 0;
onpHelper([],[H|_]) -> H;
onpHelper(["+"|T1], [H1,H2|T2]) -> onpHelper(T1, [H2+H1 | T2]);
onpHelper(["-"|T1], [H1,H2|T2]) -> onpHelper(T1, [H2-H1 | T2]);
onpHelper(["*"|T1], [H1,H2|T2]) -> onpHelper(T1, [H2*H1 | T2]);
onpHelper(["/"|T1], [H1,H2|T2]) -> onpHelper(T1, [H2/H1 | T2]);
onpHelper(["sqrt"|T1], [H1|T2]) -> onpHelper(T1, [sqrt(H1) | T2]);
onpHelper(["pow" |T1], [H1,H2|T2]) -> onpHelper(T1, [pow(H2,H1) | T2]);
onpHelper(["sin" |T1], [H1|T2]) -> onpHelper(T1, [sin(H1) | T2]);
onpHelper(["cos" |T1], [H1|T2]) -> onpHelper(T1, [cos(H1) | T2]);
onpHelper(["tg"  |T1], [H1|T2]) -> onpHelper(T1, [tan(H1) | T2]);
onpHelper(["ctg" |T1], [H1|T2]) -> onpHelper(T1, [ctg(H1) | T2]);
onpHelper(["pi" |T1], Stack) -> onpHelper(T1, [ pi() | Stack]);
onpHelper([H1|T1], Stack) ->
  case string:to_float(H1) of
    {error, no_float} -> onpHelper(T1, [list_to_integer(H1)| Stack]);
    _ -> onpHelper(T1, [list_to_float(H1)| Stack])
  end.




onp(Str) ->
  onpHelper(string:tokens(Str, " "), []).






