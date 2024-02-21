/*
    TBD:
    - It would be ideal to keep animation progress (colour ondulating, cube spin) through page refreshes so they don't seem to "reset". The way to do this would be saving the progress in session storage before unloading the page & then setting it immediately on page load.
*/

/*
    `R_8bit`, `G_8bit`, `B_8bit` in range [0, 255].
    Implemented per https://www.w3.org/TR/WCAG20-TECHS/G17.html#G17-procedure. https://stackoverflow.com/a/56678483 argues that the threshold in `convert_X_sRGB_to_X` should be slightly higher than the standard's.
    Could allow for more accent colours by lowering the contrast ratio to 4.5. This is counter to the recommendations in https://www.w3.org/TR/UNDERSTANDING-WCAG20/visual-audio-contrast-contrast.html regarding only doing so for large text, but how many recruiters with vision poorer than an average 80 year old's are likely to be interested in my CV anyway?
*/
const white = {
    R: 255,
    G: 255,
    B: 255,
};

const black = {
    R: 0,
    G: 0,
    B: 0,
};

function relative_luminance(RGB_colour_8bit) {
    const convert_X_8bit_to_X_sRGB = X_8bit => X_8bit / 255;
    const convert_X_sRGB_to_X = X_sRGB => X_sRGB <= 0.03928 ? X_sRGB / 12.92 : ((X_sRGB + 0.055) / 1.055) ** 2.4;
    const convert = X_8bit => convert_X_sRGB_to_X(convert_X_8bit_to_X_sRGB(X_8bit));

    const R = convert(RGB_colour_8bit.R);
    const G = convert(RGB_colour_8bit.G);
    const B = convert(RGB_colour_8bit.B);

    return 0.2126 * R + 0.7152 * G + 0.0722 * B;
}

const L_white = relative_luminance(white);
const L_black = relative_luminance(black);

function evaluate_contrast(background_colour) {
    const contrast_ratio = (L1, L2) => (L1 + 0.05) / (L2 + 0.05);

    const L_bg = relative_luminance(background_colour);

    const white_contrast_ratio = contrast_ratio(L_white, L_bg);
    const black_contrast_ratio = contrast_ratio(L_bg, L_black);

    const highest_contrast = white_contrast_ratio > black_contrast_ratio ? 'white' : 'black';
    return {
        enough: (highest_contrast === 'white' ? white_contrast_ratio : black_contrast_ratio) >= 7,
        which: highest_contrast,
        white_contrast_ratio,
        black_contrast_ratio,
    };
}

function generate_colour() {
    const num_between_0_incl_255_incl = () => Math.floor(Math.random() * 256);

    return {
        R: num_between_0_incl_255_incl(),
        G: num_between_0_incl_255_incl(),
        B: num_between_0_incl_255_incl(),
    }
};

/*
    The goal here is to prevent the accent colour from being so similar to the body's background colour that it becomes hard to tell where the accents (e.g., section titles) end & the body begins.

    Initially I thought to reuse the `evaluate_contrast` function to set a minimum contrast between the 2 colours, but this clearly doesn't work as expected (e.g., the contrast between cyan [sRGB 0 255 255] and white is ranked higher than the contrast between yellow [sRGB 255 255 0] and white, despite cyan and white subjectively feeling more similar than yellow and white). This raises the question of why `evaluate_contrast` is the W3C-standard-recommended way of ensuring the text contrasts (= in my understanding, is different) enough with the background. I think the answer is that the standard sets such a high threshold for the contrast ratio (7 -- for comparison, cyan/white had a ratio of ~1.16, yellow/white had a ratio of ~1.05, and the maximum ratio is white/black at ~21) that looking just at the luminance information in the CIE XYZ colour space (which is what `relative_luminance` is actually doing -- converting from sRGB to CIE XYZ, but just to get the Y) is enough to determine if letters contrast without complicating the algorithm (=> if this is correct, it means there are other colours with lower luminance Y but higher X and Z which the standard's algorithm will mark at not contrasting enough when they do in fact contrast enough) + by not relying on hue/saturation information, the algorithm is more accurate/resilient for colourblind people.

    After investigating some more, I found the Delta E* family of algorithms. Delta E* purports to find the perceptual difference between 2 given colours, so it seems that the correct way to do this would be to (1) calculate the body's background colour from the accent colour (either by reimplementing CSS's `color-mix` in JavaScript -- which I believe to be just a linear interpolation -- or by using a hidden HTML element with the appropriate CSS stylings set to do the calculation for us), (2) convert the accent colour from sRGB to CIELAB (via sRGB -> CIE XYZ; CIE XYZ -> CIELAB), (3) convert the body's background colour from sRGB to CIELAB, (4) compare colours with one of the Delta E* algorithms & set a cut-off based on experimenting with what looks pleasing to the eye. There are several Delta E* algorithms to choose from, but for this purpose I believe Delta E* 76 is probably good enough: it's comparatively simple to implement, and by construction(*) the colours I want to compare will always have a similar luminosity level -- which is a situation in which Delta E* 76 performs well.

    (*) Recall that the body's background colour is just 10% the accent colour + 90% white, and that the issue I'm trying to defend against is the accent colour sometimes blending together with the background. Given how "white" the background colour is guaranteed to be, this is only an issue when the accent colour itself is already pretty close to white -- which in my understanding guarantees the luminance of both colours will be pretty high & close.

    Alas, this all still felt a bit much to implement (at least for now), so I kept searching for something simpler now that I knew the term to look for ("colour difference"). Wikipedia mentioned a way to compute the colour difference directly in sRGB, using CompuPhase's colour metric, but experimenting with this showed it to be no better than just calculating the luminance as I'd started out doing (using this metric, both cyan and yellow have the same distance to white).

    All said and done, I ended up using the following approach: starting from an accent colour at sRGB 255 255 255 (white), how much can I reduce each colour channel until the accent colour subjectively feels different enough from the background colour, and how simple can I make the resulting formulate? The implementation of this function is the end result. An interesting note is that I'm partially red-green colourblind (malformed red cones) & that my formula naturally ended up "penalising" red (i.e., if both colours have a lot of green and blue but differing amounts of red, then they have to differ by a lot of red before I can tell they're significantly different) -- which might just be a coincidence, as the CompuPhase colour metric also has some special handling for red & Delta E* (76 and 00) both agree that cyan is closer to white than yellow is (=> meaning a maxed out blue channel contributes more to making the colour different than a maxed out red channel).

    So, TBD: if I ever have the patience, change the formula below to use one of the Delta E* algorithms (76 is probably fine) with a subjectively defined cut-off.

    References:
        https://stackoverflow.com/a/56678483 - Computing the brightness of a colour.
        http://colormine.org/delta-e-calculator - Several Delta E* calculators (this page is 76, but there's links to others).
        https://photo.stackexchange.com/questions/116548/is-cyan-brighter-than-red - What I stumbled upon initially as I tried to understand why cyan felt closer to white than yellow did despite the luminsnvr (Y in CIE XYZ) calculations saying otherwise.
        https://en.wikipedia.org/wiki/Color_difference - Reference page for colour difference calculations (Wikipedia is good for colour spaces too).
        https://www.compuphase.com/cmetric.htm - Colour distance metric directly in sRGB space, referenced by the Wikipedia page above. I do not find it to measure colour distance correctly despite what's claimed.
        https://stackoverflow.com/a/9019461 - Basically describing the ideal process I arrived at (sRGB to CIELAB, then Delta E*).
        https://zschuessler.github.io/DeltaE/learn/ - Amazing comparison of the different Delta E* variants.
        https://www.nixsensor.com/free-color-converter/ - sRGB to CIELAB colour converter (clearly showing that the luminance L* is not enough for my purposes -- yellow has ~97% luminance whereas cyan has 90%, but I find cyan to be "obviously" closer to the page's white background than yellow is).
        https://developer.mozilla.org/en-US/docs/Web/Accessibility/Understanding_Colors_and_Luminance - Only even found this while already finishing this comment, but still felt like such a good resource that I had to include it.
        https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/ - What I had originally planned to base myself on for `generate_accent_colour_pair`. The implementation is simple, but it's easy to see some of the resulting colours blend more into the white background of the page than others (e.g., letter D in the final results). This is actually kinda orthogonal to checking if colours blend into white too much (not completely orthogonal because if I were to use this method for something & simply started skipping colours that blended too much into white then I'd run into the problem of colours clustering together as I "biasedly" reject cyans more than magentas, for example), but I ultimately had no real reason to go with this (ever so slightly) more complicated way of generating colours given I only needed to generate one (no real risk of generating another one that looks too dark/light/intense next to it) and that the contrast + not-too-white contraints I had already constrained my colours to a generally aesthetically pleasing portion of the colour space.
        https://facelessuser.github.io/coloraide/distance/ - Only found it after writing this comment, but seemed like a great resource so I'm including it here.
        https://github.com/svgeesus/svgeesus.github.io/blob/master/Color/OKLab-notes.md - Likewise, only found it after writing this comment but seemed like a great resource.
*/
function colour_is_too_whitish(colour) {
    if(colour.R + colour.G + colour.B > 255 * 3 - 30) {
        return true;
    }

    if(colour.G + colour.B > 255 * 2 - 40) {
        if(colour.R > 110) {
            return true;
        }
    }

    return false;
}

function generate_accent_colour_pair() {
    let accent_colour;
    let contrast_info;

    do {
        accent_colour = generate_colour();
        contrast_info = evaluate_contrast(accent_colour);
    } while(!contrast_info.enough || colour_is_too_whitish(accent_colour));

    return [`rgb(${accent_colour.R} ${accent_colour.G} ${accent_colour.B})`, contrast_info.which];
}

function set_accent_colours([accent_colour, accent_contrast_colour]) {
    document.documentElement.style.setProperty('--accent-colour', accent_colour);
    document.documentElement.style.setProperty('--accent-contrast-colour', accent_contrast_colour);
}

//Parsing the alpha channel is just a safety guard (and ends up being needed in the `interpolatedBestTextColour` code below, although it's currently commented out). Making the commas optional is also just a safety guard, as in almost all situations RGB values have `,` when read via `.style` or `window.getComputedStyle` ("almost all" because the browser seems incapable of doing this for custom attributes -- probably for the same reason it can't do interpolation and requires the `@property` declaration --, thus the `--accent-colour` attribute on the `html` tag is read back without commas, which doesn't really matter because we never read it back). The `/` in the alpha channel is also just a safety guard, as that's also acceptable syntax for the `rgb` CSS function (although browsers will add a comma when reading back, similarly to what happens for other channels).
const rgb_regex = /rgba?\((?<R>\d+),? (?<G>\d+),? (?<B>\d+)((,|\/)? (?<A>\d+|0.\d+))?\)/;
function parse_rgb(rgb_colour) {
    const channels = rgb_colour.match(rgb_regex).groups;

    const rgb = {
        R: Number.parseInt(channels.R),
        G: Number.parseInt(channels.G),
        B: Number.parseInt(channels.B),
    };

    if(channels.A !== undefined) {
        rgb.A = Number.parseInt(channels.A);
    }

    return rgb;
}

let animationID = null;
function pick_colour(event) {
    if(event.type === 'keyup' && event.key !== 'Enter') {
        return;
    }

    if(animationID !== null) {
        window.cancelAnimationFrame(animationID);
    }

    const clickedElement = event.currentTarget;
    if(clickedElement.id === 'random_colour_picker') {
        set_accent_colours(generate_accent_colour_pair());
    } else if(clickedElement.classList.contains('ondulating_colour')) {
        //The idea here was to gracefully transition from white to black as the colours change. This ended up looking ugly and had very poor contrast at the intermediate greys. I've kept the code because it might still be useful if I ever want to revisit this idea (a sharper transition that spends less time at the intermediate greys should work better).
        /*
        //Assumes no more than one 'ondulating-colours-XXX' animation per element (reasonable)
        const ondulating_colours_animation = Array.from(clickedElement.getAnimations()).filter(anim => anim.animationName.includes('ondulating-colours'))[0];
        const idealTextColours = ondulating_colours_animation.effect.getKeyframes().map(kf => {
            let bgColour = parse_rgb(kf.backgroundColor);
            if(bgColour.A === '0') {    //Missing initial or ending %s in keyframes default to a `background-color` of `rgba(0, 0, 0, 0)` which mistakenly looks like black if you ignore the alpha channel. Not 100% ideal (the ideal solution would be to actually interpret the alpha channel).
                bgColour = white;
            }

            return {
                offset: kf.offset,
                bestTextColour: evaluate_contrast(bgColour).which,
            };
        }).sort(({offset: o1}, {offset: o2}) => o1 - o2);  //Guarantee it's sorted in ascending order (so the first offset comes first, etc)

        function interpolatedBestTextColour(animationProgress) {
            if(animationProgress === 0) {
                return idealTextColours[0].bestTextColour;
            }

            let previousTextColour, nextTextColour;
            for(let i = 1; i < idealTextColours.length; ++i) {
                if(animationProgress <= idealTextColours[i].offset) {
                    previousTextColour = idealTextColours[i - 1];
                    nextTextColour = idealTextColours[i];

                    break;
                }
            }

            if(previousTextColour.bestTextColour === nextTextColour.bestTextColour) {
                return previousTextColour.bestTextColour;
            }

            const percentOfTransition = (animationProgress - previousTextColour.offset) / (nextTextColour.offset - previousTextColour.offset);
            const from = previousTextColour.bestTextColour === 'white' ? white : black;
            const to = nextTextColour.bestTextColour === 'white' ? white : black;

            const result = {
                R: from.R + (to.R - from.R) * percentOfTransition,
                G: from.G + (to.G - from.G) * percentOfTransition,
                B: from.B + (to.B - from.B) * percentOfTransition,
            };

            return result;
        }*/

        /* TBD: It would be much more elegant (and presumably prone to hardware acceleration/better performance) to use just CSS for the colour animation. Unfortunately, Firefox does not yet support interpolating custom attributes (because it requires the `@property` rule, which is only available on Nightly at the time of writing) so there's no way to animate `--accent-colour` and `--accent-contrast-colour` on `:root` in a portable manner. Once it is available, this should be ported over onto it. (Observation: using the "define a `linear-gradient` background and have a transparent `div` move 'over' it" approach wouldn't be equivalent because the `div` would not change colour uniformly; rather it'd seem to start at one end of the `div`) */
        function colour_ondulation_step() {
            const currentColour = window.getComputedStyle(clickedElement).backgroundColor;
            const { which: mostContrastingTextColour } = evaluate_contrast(parse_rgb(currentColour));

            /*
            const interpolatedBetweenMostContrastingTextColours = interpolatedBestTextColour(ondulating_colours_animation.effect.getComputedTiming().progress);
            document.documentElement.style.setProperty('--accent-contrast-colour', `rgb(${interpolatedBetweenMostContrastingTextColours.R} ${interpolatedBetweenMostContrastingTextColours.G} ${interpolatedBetweenMostContrastingTextColours.B})`);
            */

            set_accent_colours([currentColour, mostContrastingTextColour]);
            animationID = window.requestAnimationFrame(colour_ondulation_step);
        }

        animationID = window.requestAnimationFrame(colour_ondulation_step);
    } else {
        const accent_colour = window.getComputedStyle(event.target).backgroundColor;

        const contrast_info = evaluate_contrast(parse_rgb(accent_colour));

        if(!contrast_info.enough) {
            console.warn(`Pre-defined accent colour ${accent_colour} does not contrast either possible accent contrast colour (white/black). This is probably an oversight.`);
        }

        const accent_contrast_colour = contrast_info.which;
        set_accent_colours([accent_colour, accent_contrast_colour]);
    }
}

function force_company_colour(company_colour) {
    let company_colour_obj = company_colour, company_colour_string = company_colour;

    if(typeof company_colour === 'string') {
        company_colour_obj = parse_rgb(company_colour);
    } else {
        company_colour_string = `rgb(${company_colour.R} ${company_colour.G} ${company_colour.B})`;
    }

    const contrast_info = evaluate_contrast(company_colour_obj);
    if(!contrast_info.enough) {
        console.info(`Contrast between ${company_colour_string} and ${contrast_info.which} is only ${contrast_info.which === 'black' ? contrast_info.black_contrast_ratio : contrast_info.white_contrast_ratio} (should be >= 7)`);
    }

    set_accent_colours([company_colour_string, contrast_info.which]);
}

let original_title;
window.onload = () => {
    original_title = document.title;
    set_accent_colours(generate_accent_colour_pair());

    for(const picker of document.getElementsByClassName('colour_picker')) {
        picker.addEventListener('click', pick_colour);
        picker.addEventListener('keyup', pick_colour);
    }
};

window.onbeforeprint = () => {
    document.title = 'Alvaro_Santos_CV';

    const bot_protected_text = document.getElementsByClassName('bot_protection');
    for(const bpt of bot_protected_text) {
        let replacementText = bpt.getElementsByClassName("replacement");

        if(replacementText.length !== 1) {
            console.warn(`Found too ${replacementText.length === 0 ? 'few (0)' : `many (${replacementText.length})`} replacements in a bot-protected element. Will not replace anything.`);
            continue;
        }

        const replacementElement = document.createElement('span');
        replacementElement.classList.add('bot_protection_replacement');
        replacementElement.textContent = replacementText[0].textContent;

        bpt.insertAdjacentElement('afterend', replacementElement);
        bpt.style.display = 'none';
    }
};

window.onafterprint = () => {
    document.title = original_title;

    const bot_protected_text = document.getElementsByClassName('bot_protection');
    Array.from(bot_protected_text).forEach(bpt => bpt.style.removeProperty('display'));

    const bot_protection_replacements = document.getElementsByClassName('bot_protection_replacement');
    Array.from(bot_protection_replacements).forEach(bpr => bpr.remove());
};
