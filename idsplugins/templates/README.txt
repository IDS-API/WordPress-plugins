The following template tags are available to use in templates.

They return HTML text with the information associated with the current post in the loop, if it represents an IDS document or organisation.
Category terms are linked to their respective term listing pages.

- ids_date_updated($before, $after)
- ids_countries($before, $sep, $after)
- ids_regions($before, $sep, $after)
- ids_themes($before, $sep, $after)

* For documents' templates:

- ids_authors($before, $sep, $after) 
- ids_external_urls($before, $sep, $after)

* For organisations' templates:

- ids_acronym($before, $after)
- ids_location_country($before, $after)
- ids_organisation_type($before, $after)
- ids_organisation_url($before, $after)

Parameters:

- $before (string) (optional) Leading text. Default: None
- $sep (string) (optional) String to separate tags. Default: ","
- $after (string) (optional) Trailing text. Default: None
