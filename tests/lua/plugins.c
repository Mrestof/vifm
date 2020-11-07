#include <stic.h>

#include "../../src/cfg/config.h"
#include "../../src/lua/vlua.h"
#include "../../src/ui/statusbar.h"
#include "../../src/utils/str.h"
#include "../../src/plugins.h"

#include <test-utils.h>

static vlua_t *vlua;
static plugs_t *plugs;
static plug_t plug_dummy;

SETUP()
{
	conf_setup();

	make_abs_path(cfg.config_dir, sizeof(cfg.config_dir), SANDBOX_PATH, "", NULL);
	create_dir(SANDBOX_PATH "/plugins");
	create_dir(SANDBOX_PATH "/plugins/plug");

	vlua = vlua_init();
	plugs = plugs_create(vlua);
}

TEARDOWN()
{
	conf_teardown();

	plugs_free(plugs);
	vlua_finish(vlua);

	remove_dir(SANDBOX_PATH "/plugins/plug");
	remove_dir(SANDBOX_PATH "/plugins");

	update_string(&plug_dummy.log, NULL);
	plug_dummy.log_len = 0;
}

TEST(good_plugin_loaded)
{
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "return {}");

	ui_sb_msg("");
	assert_success(vlua_load_plugin(vlua, "plug", &plug_dummy));
	assert_string_equal("", ui_sb_last());

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
}

TEST(bad_return_value)
{
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "return 123");

	ui_sb_msg("");
	assert_failure(vlua_load_plugin(vlua, "plug", &plug_dummy));
	assert_string_equal("Failed to load 'plug' plugin: it didn't return a table",
			ui_sb_last());

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
}

TEST(syntax_error)
{
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "-+");

	ui_sb_msg("");
	assert_failure(vlua_load_plugin(vlua, "plug", &plug_dummy));
	assert_true(starts_with_lit(ui_sb_last(), "Failed to load 'plug' plugin: "));

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
}

TEST(runtime_error)
{
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "badcall()");

	ui_sb_msg("");
	assert_failure(vlua_load_plugin(vlua, "plug", &plug_dummy));
	assert_true(starts_with_lit(ui_sb_last(), "Failed to start 'plug' plugin: "));

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
}

TEST(multiple_plugins_loaded)
{
	create_dir(SANDBOX_PATH "/plugins/plug2");
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "return {}");
	make_file(SANDBOX_PATH "/plugins/plug2/init.lua", "return {}");

	ui_sb_msg("");
	plugs_load(plugs, cfg.config_dir);
	assert_string_equal("", ui_sb_last());

	assert_success(vlua_run_string(vlua,
	      "print((vifm.plugins.all.plug and '1y' or '1n').."
	      "      (vifm.plugins.all.plug2 and '2y' or '2n'))"));
	assert_string_equal("1y2y", ui_sb_last());

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
	remove_file(SANDBOX_PATH "/plugins/plug2/init.lua");
	remove_dir(SANDBOX_PATH "/plugins/plug2");
}

TEST(loading_missing_plugin_fails)
{
	assert_failure(vlua_load_plugin(vlua, "plug", &plug_dummy));
}

TEST(plugin_statuses_are_correct)
{
	create_dir(SANDBOX_PATH "/plugins/plug2");
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "return {}");
	make_file(SANDBOX_PATH "/plugins/plug2/init.lua", "return");

	plugs_load(plugs, cfg.config_dir);

	const plug_t *plug;
	PluginLoadStatus status;
	assert_true(plugs_get(plugs, 0, &plug));
	status = (ends_with(plug->path, "plug2") ? PLS_FAILURE : PLS_SUCCESS);
	assert_int_equal(status, plug->status);
	assert_true(plugs_get(plugs, 1, &plug));
	status = (ends_with(plug->path, "plug2") ? PLS_FAILURE : PLS_SUCCESS);
	assert_int_equal(status, plug->status);
	assert_false(plugs_get(plugs, 2, &plug));

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
	remove_file(SANDBOX_PATH "/plugins/plug2/init.lua");
	remove_dir(SANDBOX_PATH "/plugins/plug2");
}

TEST(can_not_load_same_plugin_twice, IF(not_windows))
{
	make_file(SANDBOX_PATH "/plugins/plug/init.lua", "return {}");
	assert_success(make_symlink("plug", SANDBOX_PATH "/plugins/plug2"));

	plugs_load(plugs, cfg.config_dir);

	const plug_t *plug1, *plug2, *plug3;
	assert_true(plugs_get(plugs, 0, &plug1));
	PluginLoadStatus status1 = plug1->status;
	assert_true(plugs_get(plugs, 1, &plug2));
	PluginLoadStatus status2 = plug2->status;
	assert_false(plugs_get(plugs, 2, &plug3));

	assert_true((status1 == PLS_SUCCESS && status2 == PLS_SKIPPED) ||
	            (status2 == PLS_SUCCESS && status1 == PLS_SKIPPED));

	const plug_t *skipped = (status1 == PLS_SKIPPED ? plug1 : plug2);
	assert_true(starts_with_lit(skipped->log,
				"[vifm][error]: skipped as a duplicate of"));

	remove_file(SANDBOX_PATH "/plugins/plug/init.lua");
	remove_file(SANDBOX_PATH "/plugins/plug2");
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
