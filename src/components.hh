
#pragma once

struct DavidLTPort : public SVGPort {
   DavidLTPort() {
      setSVG(APP->window->loadSvg(assetPlugin(::plugin, "res/cntr_LT.svg")));
   }
};
