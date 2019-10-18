# Thermal Conductivity Calculation with MPI

*Collaborators: Nazariy Bachynsky & Danylo Kolinko*

Configuration files **should** be named `config.dat` and `heat_map.conf`<br/>
Where *config.dat* has the following attributes:
```
conductivity = a
capacity = b
density = c
gridX = d
gridY = e
deltaX = f
deltaY = g
deltaT = h
printT = i
time = g
```
*a-g* are some numbers<br>
`printT` - is interval with which result of calculation should be saved<br>
Unit of measurement of `deltaT`, `printT` and `time` is second<br>
As we see, there is delimiter '=', 
it can be configured in main file when reading configurations with `read_conf()`<br>

The structure of `heat_map.conf` is:
```
n1  :  t1
n2  :  t2
.   :  .
'   :  '
n_k : t_k
``` 
(Again, here we have delimiter ':' required)<br>
`n_k : t_k` means that *n* points on perimeter (clockwise) will have value *t*, like this:<br>
```
t1 t1 t1 t1 t2 t2
t5             t2
t5             t2
t4             t2
t4             t2
t3 t3 t3 t3 t3 t3
```
*Remark*: if sizes of grid perimeter given in these two files: `gridX * 2 + gridY * 2 - 4` and 
`n1 + n2 + ... + n_k` doesn't equal each other you'll get an error<br>

To make a gif from saved intermediate results run file `make_gif.py` (**package imageio is required**), the name of the result file is `heat_map_py.gif`<br>
or `make_gif.sh` (is slower) - `heat_map_sh.gif`