use anyhow::{Context, Result};
use clap::{Parser, Subcommand};
use console::Style;
use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

#[derive(Parser)]
#[command(name = "example-maker")]
#[command(about = "Raylib ESP-IDF component build and test tool")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
    #[arg(short, long)]
    verbose: bool,
}

#[derive(Subcommand)]
enum Commands {
    /// Regenerate all examples from template
    Regenerate {
        #[arg(long)]
        template: Option<PathBuf>,
        #[arg(long)]
        output: Option<PathBuf>,
    },
    /// Build all examples
    Build {
        #[arg(long)]
        examples: Option<PathBuf>,
        #[arg(short, long, default_value = "4")]
        jobs: usize,
    },
    /// List all examples
    List {
        #[arg(long)]
        examples: Option<PathBuf>,
    },
}

#[derive(Clone, Debug)]
struct BoardInfo {
    template_option: String,
    chip: String,
    vendor_board: String,
    display_name: String,
}

fn main() -> Result<()> {
    let cli = Cli::parse();
    let green = Style::new().green();
    let red = Style::new().red();

    let result = match cli.command {
        Commands::Regenerate { template, output } => cmd_regenerate(template, output),
        Commands::Build { examples, jobs } => cmd_build(examples, jobs, cli.verbose),
        Commands::List { examples } => cmd_list(examples),
    };

    if let Err(e) = result {
        eprintln!("{}", red.apply_to(format!("{e}")));
        std::process::exit(1);
    }
    Ok(())
}

#[derive(Clone, Copy)]
struct BlockKind {
    current: bool,
    any: bool,
}

impl BlockKind {
    fn include_line(self) -> bool {
        match self {
            BlockKind { current: true, .. } => !self.any,
            _ => false,
        }
    }

    fn new_if(current: bool) -> BlockKind {
        BlockKind {
            current,
            any: false,
        }
    }

    fn into_else_if(self, condition: bool) -> BlockKind {
        let (current, any) = match self {
            BlockKind { current, any } => (current, any),
        };
        BlockKind {
            current: condition,
            any: any || current,
        }
    }

    fn into_else(self) -> BlockKind {
        let (current, any) = match self {
            BlockKind { current, any } => (current, any),
        };
        BlockKind {
            current: !any,
            any: any || current,
        }
    }
}

fn process_file(
    contents: &str,
    options: &[String],
    variables: &[(String, String)],
    file_path: &mut String,
) -> Option<String> {
    let mut res = String::new();
    let mut replace: Option<Vec<(String, String)>> = None;
    let mut include = vec![BlockKind::new_if(true)];
    let mut file_directives = true;

    let mut engine = somni_expr::Context::new();
    engine.add_function("option", move |cond: &str| -> bool {
        options.iter().any(|c| c == cond)
    });

    let mut include_file = true;

    for (line_no, line) in contents.lines().enumerate() {
        let line_no = line_no + 1;
        let trimmed = line.trim();

        if file_directives {
            if let Some(cond) = trimmed
                .strip_prefix("//INCLUDEFILE ")
                .or_else(|| trimmed.strip_prefix("#INCLUDEFILE "))
                .or_else(|| trimmed.strip_prefix("--INCLUDEFILE "))
            {
                include_file = engine.evaluate::<bool>(cond).unwrap();
                continue;
            } else if let Some(include_as) = trimmed
                .strip_prefix("//INCLUDE_AS ")
                .or_else(|| trimmed.strip_prefix("#INCLUDE_AS "))
                .or_else(|| trimmed.strip_prefix("--INCLUDE_AS "))
            {
                *file_path = include_as.trim().to_string();
                continue;
            }
        }
        if !include_file {
            return None;
        }
        file_directives = false;

        if let Some(what) = trimmed
            .strip_prefix("#REPLACE ")
            .or_else(|| trimmed.strip_prefix("//REPLACE "))
            .or_else(|| trimmed.strip_prefix("--REPLACE "))
        {
            let replacements = what
                .split(" && ")
                .filter_map(|pair| {
                    let mut parts = pair.split_whitespace();
                    if let (Some(pattern), Some(var_name)) = (parts.next(), parts.next()) {
                        variables
                            .iter()
                            .find(|(key, _)| key == var_name)
                            .map(|(_, value)| (pattern.to_string(), value.clone()))
                    } else {
                        None
                    }
                })
                .collect::<Vec<_>>();
            if !replacements.is_empty() {
                replace = Some(replacements);
            }
        } else if trimmed.starts_with("#IF ")
            || trimmed.starts_with("//IF ")
            || trimmed.starts_with("--IF ")
        {
            let cond = trimmed
                .strip_prefix("#IF ")
                .or_else(|| trimmed.strip_prefix("//IF "))
                .or_else(|| trimmed.strip_prefix("--IF "))
                .unwrap();
            let last = *include.last().unwrap();
            let current = if last.include_line() {
                engine.evaluate::<bool>(cond).unwrap()
            } else {
                false
            };
            include.push(BlockKind::new_if(current));
        } else if trimmed.starts_with("#ELIF ")
            || trimmed.starts_with("//ELIF ")
            || trimmed.starts_with("--ELIF ")
        {
            let cond = trimmed
                .strip_prefix("#ELIF ")
                .or_else(|| trimmed.strip_prefix("//ELIF "))
                .or_else(|| trimmed.strip_prefix("--ELIF "))
                .unwrap();
            let last = include.pop().unwrap();
            let current = if matches!(
                last,
                BlockKind {
                    current: false,
                    any: false
                }
            ) {
                engine.evaluate::<bool>(cond).unwrap()
            } else {
                false
            };
            include.push(last.into_else_if(current));
        } else if trimmed.starts_with("#ELSE")
            || trimmed.starts_with("//ELSE")
            || trimmed.starts_with("--ELSE")
        {
            let last = include.pop().unwrap();
            include.push(last.into_else());
        } else if trimmed.starts_with("#ENDIF")
            || trimmed.starts_with("//ENDIF")
            || trimmed.starts_with("--ENDIF")
        {
            let prev = include.pop();
            assert!(prev.is_some(), "ENDIF without IF in {file_path}:{line_no}");
        } else if include.iter().all(|v| v.include_line()) {
            let mut line = line.to_string();
            if trimmed.starts_with("#+") {
                line = line.replace("#+", "");
            }
            if trimmed.starts_with("//+") {
                line = line.replace("//+", "");
            }
            if trimmed.starts_with("--+") {
                line = line.replace("--+", "");
            }
            if let Some(replacements) = &replace {
                for (pattern, value) in replacements {
                    line = line.replace(pattern, value);
                }
            }
            res.push_str(&line);
            res.push('\n');
            replace = None;
        }
    }

    Some(res)
}

fn read_template_files(template_dir: &Path) -> Result<Vec<(String, String)>> {
    let mut files = Vec::new();
    fn walk(dir: &Path, base: &Path, out: &mut Vec<(String, String)>) {
        for entry in fs::read_dir(dir).expect("read dir") {
            let entry = entry.unwrap();
            let path = entry.path();
            if path.is_dir() {
                walk(&path, base, out);
            } else if path.is_file()
                && path.file_name().and_then(|f| f.to_str()) != Some("template.yaml")
            {
                let rel = path
                    .strip_prefix(base)
                    .unwrap()
                    .display()
                    .to_string()
                    .replace('\\', "/");
                let content = fs::read_to_string(&path).unwrap();
                out.push((rel, content));
            }
        }
    }
    walk(template_dir, template_dir, &mut files);
    Ok(files)
}

fn generate_one(
    template_dir: &Path,
    chip: &str,
    option: &str,
    name: &str,
    output_dir: &Path,
) -> Result<()> {
    let chip_output_dir = output_dir.join(chip);
    fs::create_dir_all(&chip_output_dir)?;
    let final_output_dir = chip_output_dir.join(name);
    if final_output_dir.exists() {
        fs::remove_dir_all(&final_output_dir)?;
    }

    let files = read_template_files(template_dir)?;
    let selected = vec![option.to_string()];
    let variables = [("project-name".to_string(), name.to_string())];

    for (rel_path, contents) in files {
        let mut file_path = rel_path.clone();
        match process_file(&contents, &selected, &variables, &mut file_path) {
            Some(processed) => {
                let dest = final_output_dir.join(file_path);
                fs::create_dir_all(dest.parent().unwrap())?;
                fs::write(dest, processed)?;
            }
            None => {}
        }
    }

    for script in ["run-wokwi-local.sh", "wokwi-run.sh"] {
        let p = final_output_dir.join(script);
        if p.exists() {
            use std::os::unix::fs::PermissionsExt;
            let perms = fs::metadata(&p)?.permissions();
            let mut perms = perms.mode();
            perms |= 0o755;
            fs::set_permissions(&p, std::fs::Permissions::from_mode(perms))?;
        }
    }

    Ok(())
}

fn get_boards() -> Vec<BoardInfo> {
    vec![
        BoardInfo {
            template_option: "esp32_s3_box_3".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp32-s3-box-3".into(),
            display_name: "ESP32-S3-BOX-3".into(),
        },
        BoardInfo {
            template_option: "esp32_s3_box".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp32-s3-box".into(),
            display_name: "ESP32-S3-BOX".into(),
        },
        BoardInfo {
            template_option: "esp32_s3_lcd_ev_board".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp32-s3-lcd-ev-board".into(),
            display_name: "ESP32-S3-LCD-EV".into(),
        },
        BoardInfo {
            template_option: "esp32s3_korvo_2".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp32-s3-korvo-2".into(),
            display_name: "ESP32-S3-Korvo-2".into(),
        },
        BoardInfo {
            template_option: "esp_vocat".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp-vocat".into(),
            display_name: "ESP-VoCat".into(),
        },
        BoardInfo {
            template_option: "esp32_s3_eye".into(),
            chip: "esp32s3".into(),
            vendor_board: "espressif-esp32-s3-eye".into(),
            display_name: "ESP32-S3-EYE".into(),
        },
        BoardInfo {
            template_option: "m5stack_core_s3".into(),
            chip: "esp32s3".into(),
            vendor_board: "m5stack-core-s3".into(),
            display_name: "M5Stack-CoreS3".into(),
        },
        BoardInfo {
            template_option: "m5stack_atom_s3".into(),
            chip: "esp32s3".into(),
            vendor_board: "m5stack-atom-s3".into(),
            display_name: "M5Stack-AtomS3".into(),
        },
        BoardInfo {
            template_option: "m5stack_atom_s3r".into(),
            chip: "esp32s3".into(),
            vendor_board: "m5stack-atom-s3r".into(),
            display_name: "M5Stack-AtomS3R".into(),
        },
        BoardInfo {
            template_option: "m5stack_core_2".into(),
            chip: "esp32".into(),
            vendor_board: "m5stack-core2".into(),
            display_name: "M5Stack-Core2".into(),
        },
    ]
}

fn cmd_regenerate(template: Option<PathBuf>, output: Option<PathBuf>) -> Result<()> {
    let template_dir = template.unwrap_or_else(|| PathBuf::from("templates/raylib-hello-c"));
    let output_dir = output.unwrap_or_else(|| PathBuf::from("examples"));

    if !template_dir.exists() {
        eprintln!(
            "Template directory '{}' not found in the current working directory.",
            template_dir.display()
        );
        eprintln!(
            "Run this tool from the directory containing templates/, or pass --template <dir>."
        );
        std::process::exit(1);
    }

    println!("Template: {}", template_dir.display());
    println!("Output:   {}", output_dir.display());
    println!();

    let boards = get_boards();
    let mut generated = 0;
    let mut failed = 0;

    for board in &boards {
        println!("Generating: {} ({})", board.display_name, board.chip);
        let name = format!("{}_hello", board.vendor_board);
        match generate_one(
            &template_dir,
            &board.chip,
            &board.template_option,
            &name,
            &output_dir,
        ) {
            Ok(()) => {
                println!("  OK");
                generated += 1;
            }
            Err(e) => {
                println!("  FAILED: {e}");
                failed += 1;
            }
        }
    }

    println!();
    println!("Summary: {} successful, {} failed", generated, failed);
    Ok(())
}

fn find_example_dirs(examples_dir: &Path) -> Result<Vec<PathBuf>> {
    let mut examples = Vec::new();
    for chip_dir in fs::read_dir(examples_dir).with_context(|| {
        format!(
            "Failed to read examples directory: {}",
            examples_dir.display()
        )
    })? {
        let chip_dir = chip_dir?;
        if !chip_dir.file_type()?.is_dir() {
            continue;
        }
        let chip_path = chip_dir.path();
        if chip_path.join("main").exists() && chip_path.join("CMakeLists.txt").exists() {
            continue;
        }
        for entry in fs::read_dir(chip_dir.path())? {
            let entry = entry?;
            let path = entry.path();
            if path.is_dir() && path.join("CMakeLists.txt").exists() {
                examples.push(path);
            }
        }
    }
    examples.sort();
    Ok(examples)
}

fn cmd_build(examples: Option<PathBuf>, jobs: usize, _verbose: bool) -> Result<()> {
    let examples_dir = examples.unwrap_or_else(|| PathBuf::from("examples"));
    let example_dirs = find_example_dirs(&examples_dir)?;

    println!(
        "Building {} examples with {} parallel jobs",
        example_dirs.len(),
        jobs
    );
    let mut failed = 0;
    for example_dir in &example_dirs {
        match Command::new("idf.py")
            .arg("build")
            .current_dir(example_dir)
            .output()
        {
            Ok(out) if out.status.success() => {}
            _ => {
                println!("Failed: {}", example_dir.display());
                failed += 1;
            }
        }
    }
    println!(
        "Build results: {} successful, {} failed",
        example_dirs.len() - failed,
        failed
    );
    Ok(())
}

fn cmd_list(examples: Option<PathBuf>) -> Result<()> {
    let examples_dir = examples.unwrap_or_else(|| PathBuf::from("examples"));
    let example_dirs = find_example_dirs(&examples_dir)?;
    println!("Found {} examples:", example_dirs.len());
    for dir in &example_dirs {
        let name = dir
            .file_name()
            .and_then(|n| n.to_str())
            .unwrap_or("unknown");
        println!("  - {}", name);
    }
    Ok(())
}
