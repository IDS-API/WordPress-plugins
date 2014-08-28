<?php
get_header(); ?>
<div class="section group main_content">
<div class="col span_3_of_4 padding10">
<!-- Call the API and populate the loop with IDS documents -->
<?php idsview_assets('eldis', 'documents'); ?>
<?php if ( have_posts() ) : ?>
  <?php if ( !idsview_is_single() ) { ?>
    <header class="archive-header">
      <h1 class="archive-title">
      <?php idsview_category_title('eldis', 'Results for: ');	?>
      </h1>
    </header><!-- .archive-header -->
  <?php } ?>
  <div class="article">				
	<?php /* The loop */ ?>
	<?php while ( have_posts() ) : the_post(); ?>
		<?php get_template_part( 'content-ids_documents', get_post_format() ); ?>
	<?php endwhile; ?>
	<?php twentythirteen_paging_nav(); ?>
  </div><!--/article-->
<?php else : ?>
	<?php get_template_part( 'content', 'none' ); ?>
<?php endif; ?>
</div><!--/col span_3_of_4-->
<div class="col span_1_of_4 sidebar padding10">
<?php if ( function_exists( 'dynamic_sidebar' ) ) dynamic_sidebar( "news-sidebar" ); ?> 
</div><!--/col span_1_of_4-->
</div><!--/section group-->
<?php get_footer(); ?>