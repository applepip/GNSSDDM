import graph;

string base_path = "d:/tmp/snacs-plot-test/txt_signals/0/";
 
real frame_width = 12.cm;
real frame_height = 4.cm;
real y_position_offset = 1.1frame_height; 
real dpi = 300;

settings.outformat="pdf";

file fglobparams = input("global-params.inc", check=false);
if (error(fglobparams) == false) {
  close(fglobparams);
  eval("include 'global-params.inc'", embedded=true);
};

int nof_points = 0;
if (frame_width > frame_height) {
  nof_points = round(dpi * (frame_width/cm) / 2.54);
} else {
  nof_points = round(dpi * (frame_height/cm) / 2.54);
}

write("nof_points: ", nof_points);

// SIGNAL GENERATOR plot
picture pic_sig_gen;
size(pic_sig_gen, frame_width, frame_height, IgnoreAspect);

file in = input(base_path + "signal-generator.txt").line();
//string x_label = in;
string x_label = "Time [$\mu$s]";
real[] x = in;
write("x-vector dimension: ", x.length);
string y_label = "Amplitude";
real[] y = in;
write("y-vector dimension: ", x.length);

marker mark = marker(scale(.5mm)*unitcircle, black, Fill); 
draw(pic_sig_gen, graph(x / 1e-6, y), mark);

ylimits(pic_sig_gen, -2, 2);
xaxis(pic_sig_gen, x_label, BottomTop, LeftTicks, above=true);
yaxis(pic_sig_gen, y_label, LeftRight, RightTicks, above=true);

// Fit pic to W of origin:
add(pic_sig_gen.fit(), (0,0), N);

// SDR plot
// early signal
picture pic_sdr;
size(pic_sdr, frame_width, frame_height, IgnoreAspect);

file in_early = input(base_path + "sdr_code_early.txt").line();
real[] x_early = in_early;
real[] y_early = in_early;

file in_late = input(base_path + "sdr_code_late.txt").line();
real[] x_late = in_late;
real[] y_late = in_late;

marker mark_early = marker(scale(.5mm)*unitcircle, green, Fill); 
marker mark_late = marker(scale(.5mm)*unitcircle, red, Fill); 
draw(pic_sdr, graph(x_early / 1e-6, y_early), green, mark_early);
draw(pic_sdr, graph(x_late / 1e-6, y_late), red, mark_late);

ylimits(pic_sdr, -2, 2);
xaxis(pic_sdr, "Time [$\mu$s]", BottomTop, LeftTicks, above=true);
yaxis(pic_sdr, "Amplitude", LeftRight, RightTicks, above=true);

// Fit pic to S of origin:
add(pic_sdr.fit(), (0, -y_position_offset), N);

// correlation points
picture pic_corr;
size(pic_corr, frame_width, frame_height, IgnoreAspect);

file in_corr = input(base_path + "sdr_correlation_points.txt").line();
real[] x_corr = in_corr;
real[] y_corr = in_corr;

marker mark_corr = marker(scale(.5mm)*unitcircle, black, Fill); 
draw(pic_corr, graph(x_corr, y_corr), black, mark_corr);

//ylimits(pic_corr, -2, 2);
xaxis(pic_corr, "Time [chips]", BottomTop, LeftTicks, above=true);
yaxis(pic_corr, "Amplitude", LeftRight, RightTicks, above=true);

// EARLY PROMPT LATE, large circles

file in_corr_epl = input(base_path + "sdr_correlation_epl.txt").line();
real[] x_corr_epl = in_corr_epl;
real[] y_corr_epl = in_corr_epl;

dot(pic_corr, (x_corr_epl[0], y_corr_epl[0]), linewidth(3mm)+green);
dot(pic_corr, (x_corr_epl[2], y_corr_epl[2]), linewidth(3mm)+red);

// Fit pic to S of origin:
add(pic_corr.fit(), (0, -2y_position_offset), N);

write("all done.");
