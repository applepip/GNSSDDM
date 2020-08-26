function save_figure_as_png(file, width, height)

set(gcf,'paperunits', 'inches');
set(gcf,'paperpos',[0 0 width height]);
set(gcf,'PaperSize',[width height]);

print('-r1','-dpng', file);

end