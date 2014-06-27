use <MCAD/boxes.scad>

export_hanger = true;
// export_base = true;
export = export_hanger || export_base;

s = [ 76, 23, 20 ];
t = 2;
to = 1.5;
w = 30;
h = 40;
r = 10;

$fs=0.05;

module hanger() {
    translate([0,0,h/2-s.z/2-0.05]) union() {
        difference() {   
            cube(size=[w,s.y+3*t+to+0.05,h],center = true);
            translate([0,0,t]) cube(size=[w+0.1,s.y+t+to,h],center=true);
            translate([0,t,s.z+0.1]) cube(size=[w+1,s.y+t+to+1,h],center=true);
        }
        translate([0,-r-(s.y+3*t+to)/2,h/2]) union() {
            rotate([0,90,0]) difference() {
                cylinder(r=r+t,h=w,center=true);
                cylinder(r=r,h=w+0.1,center=true);
                translate([r+t+0.1,0,0]) cube(size=[2*(r+t),2*(r+t),w+0.1],center=true);
            }
            translate([0,-r-t/2,-r/2]) cube(size=[w,t,r],center=true);
        }
    }
}

module base() {
    difference() {
        roundedBox(size=s + [2*t,3*t+to,0], radius=2, sidesonly=true);
        cube(size=s+[0.1,0.1,0.1], center=true);
        // cube(size=s+[-20,0,3*t],center = true);
        hanger();
    }
}

if(!export) {
    hanger();
    base();
}

if(export_hanger)
    rotate([0,90,0]) hanger();

if(export_base)
    rotate([0,180,0]) base();
