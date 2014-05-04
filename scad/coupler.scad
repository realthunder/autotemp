include <MCAD/nuts_and_bolts.scad>
include <MCAD/boxes.scad>

D=23; //trunk body diameter
sw=3; //screw diameter
t=2; //wall thick
s=5; //sink
wt=1.57; //trunk wire thick
wd=4.6; //trunk wire connector ring inner diameter
wD=8.1; //trunk wire connector ring outer diameter
w=5.5; //leave space for wire out
vd=14; //servo mount diameter
to=0.05; //screw tolerance

servoSize = [23,12,16];
servoLength = 33;
servoSpace = 2;
servoScrewL = 5;
servoScrewD = 1;

motorWireD = 4;
motorWireZ = 6;

$fs=0.05;

h=2*t+wt+s; //overall height
R=D/2+t; //outer radius


difference() {
    union() {
        hull() {
            translate([0,0,-h+s]) cylinder(r=R,h=0.01);
            translate([0,0,servoSize.z+servoSpace])   
                roundedBox(size=[servoLength,servoSize.y+2,0.01],radius=2,sidesonly=true);
        }
        translate([0,0,-h])  cylinder(r=R,h=s);
    }
    translate([0,0,-h-0.05]) {
        cylinder(r=D/2,h=s+0.1);
        cylinder(r=wD/2,h=h+0.1);
        roundedBox(size=[3.5,wD+2,2*h+0.1],radius=1,sidesonly=true);
        translate([vd/2,0,0]) cylinder(r=sw/2-0.1,h=h+0.1);
        translate([vd/2,0,h-wt]) hull() {
            cylinder(r=wD/2+0.5,h=wt+0.1);
            translate([-vd/2,0,0]) cylinder(r=wD/5,h=wt+0.1);
        }
    }
    translate([0,0,servoSize.z/2+servoSpace]) cube(servoSize+[2,1,2*servoSpace+0.1],center=true);
    for(a=[90:180:270]) {
        rotate([0,0,a]) {
            translate([0,servoLength/2-(servoLength-servoSize.x)/4,servoSize.z+servoSpace-servoScrewL]) 
                cylinder(r=servoScrewD/2,h=servoScrewL+0.05);
        }
    }
    translate([-R,0,motorWireZ+servoSpace]) rotate([0,90,0]) cylinder(r=motorWireD/2,h=10,center=true);
    translate([-R,0,(motorWireZ+servoSpace)/2]) cube([10,motorWireD,motorWireZ+servoSpace+0.1],center=true);
}




