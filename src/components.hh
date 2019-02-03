
#pragma once

struct DavidLTPort : public SVGPort {
   DavidLTPort() {	
      setSVG(SVG::load(assetPlugin(plugin, "res/cntr_LT.svg")));
   }
};
