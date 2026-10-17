#include <stdint.h>
#include "harness.h"
#include "a20_srgb.h"
#include "uicolor.h"

static void	contrast_plaques_variables(void)
{
	uint32_t	label;
	uint32_t	prompt;

	label = blend_over(UI_LABEL_PLATE, 0xffffff);
	prompt = blend_over(UI_PROMPT_PLATE, 0xffffff);
	h_true(ratio_x100(0xffffff, label) >= 450,
		"libelle d'icone: blanc sur plaque, pire fond blanc");
	h_true(ratio_x100(0xffffff, prompt) >= 450,
		"invite de connexion: blanc sur plaque, pire fond blanc");
}

int	main(void)
{
	h_begin("a20/contrast-plates");
	h_run("contraste: plaques sur fond variable", contrast_plaques_variables);
	return (h_end());
}
