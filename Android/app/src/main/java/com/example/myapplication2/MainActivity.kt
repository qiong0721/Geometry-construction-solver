package com.example.myapplication2
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import kotlin.math.sqrt
import kotlin.math.max
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.material.icons.filled.Replay
import androidx.compose.material.icons.filled.SkipNext
import androidx.compose.material.icons.filled.SkipPrevious
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.Divider
import androidx.compose.foundation.layout.Arrangement
//import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Menu
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.ui.graphics.Color
import androidx.compose.foundation.gestures.Orientation
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.remote.creation.compose.state.pow
import androidx.compose.ui.Alignment
//import androidx.compose.material3.rememberSplitPaneState
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.drawscope.translate
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlin.math.abs
import kotlin.math.pow

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    GeometrySolverApp()
                }
            }
        }
    }
}

enum class WizardStep {
    STEP_LIMIT, STEP_TARGET_TYPE, STEP_TARGET_DATA,
    STEP_COND_POINT_COUNT, STEP_COND_POINT_DATA,
    STEP_COND_LINE_COUNT, STEP_COND_LINE_DATA,
    STEP_COND_CIRCLE_COUNT, STEP_COND_CIRCLE_DATA
}

data class Point(val x: Double, val y: Double)


data class Line(val a: Double, val b: Double, val c: Double)


data class Circle(val centerX: Double, val centerY: Double, val radius: Double)


data class SolutionStep(
    val tool: String,
    val p1Index: Int,
    val p2Index: Int
)


data class Solution(
    val steps: List<SolutionStep>
)
data class FinResult(
    val stepResult: Array<ResultStep>,
    val goalmode: Int,
    val targetP: DoubleArray,
    val targetL: DoubleArray,
    val targetC: DoubleArray,
    val initP: List<Point>,
    val initL: List<Line>,
    val initC: List<Circle>
)


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GeometrySolverApp() {
    var selectedMode by remember { mutableStateOf("尺规作图") }
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var showDialog by remember { mutableStateOf(false) }
    var points by remember { mutableStateOf(listOf<Offset>()) }
    var lines by remember { mutableStateOf(listOf<Triple<Float, Float, Float>>()) }
    var circles by remember { mutableStateOf(listOf<Triple<Float, Float, Float>>()) }
    var drawSteps by remember { mutableStateOf(emptyArray<ResultStep>()) }
    var isPlaying by remember { mutableStateOf(false) }

    var goalnote: Int =-1;
    var initP by remember{mutableStateOf(listOf<Point>())}
    var initL by remember{mutableStateOf(listOf<Line>())}
    var initC by remember{mutableStateOf(listOf<Circle>())}
    var targe by remember { mutableStateOf(doubleArrayOf()) }
    var scale by remember { mutableStateOf(1f) }

    var offset by remember { mutableStateOf(Offset.Zero) }
    var currentStepIndex by remember { mutableStateOf(0) }

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            ModalDrawerSheet {
                Text("选择作图模式", modifier = Modifier.padding(16.dp), style = MaterialTheme.typography.titleMedium)
                HorizontalDivider()
                listOf("尺规作图", "单尺作图", "单规作图").forEach { mode ->
                    NavigationDrawerItem(
                        label = { Text(mode) },
                        selected = selectedMode == mode,
                        onClick = { selectedMode = mode; scope.launch { drawerState.close() } },
                        modifier = Modifier.padding(NavigationDrawerItemDefaults.ItemPadding)
                    )
                }
            }
        }
    ) {
        // 自动播放逻辑
        LaunchedEffect(isPlaying, currentStepIndex, drawSteps.size) {
            if (isPlaying && currentStepIndex <= drawSteps.size - 1) {
                kotlinx.coroutines.delay(1000)
                currentStepIndex++
            }
            else if (currentStepIndex > drawSteps.size - 1) {
                isPlaying = false
            }
        }

        Scaffold(
            topBar = {
                TopAppBar(
                    title = { Text(selectedMode) },
                    navigationIcon = {
                        IconButton(onClick = { scope.launch { drawerState.open() } }) {
                            Icon(Icons.Default.Menu, contentDescription = "菜单")
                        }
                    }
                )
            },
            floatingActionButton = {
                FloatingActionButton(onClick = { showDialog = true }) {
                    Icon(Icons.Default.Add, contentDescription = "新建")
                }
            }
        ) {
                paddingValues ->

            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(paddingValues)
            ) {

                Canvas(
                    modifier = Modifier
                        .weight(2f)
                        .fillMaxWidth()
                        .border(1.dp, Color.Gray)

                        .pointerInput(Unit) {
                            detectTransformGestures { _, pan, zoom, _ ->
                                // 更新缩放比例，限制在 0.5f 到 5f 之间
                                scale = (scale * zoom).coerceIn(0.5f, 5f)
                                // 更新平移偏移量
                                offset += pan
                            }
                        }


                ) {
                    val baseScale = 20f


                    val canvasCenter = Offset(size.width / 2f, size.height / 2f)


                    val finalScale = baseScale * scale
                    val finalOffset = canvasCenter + offset

                    fun toScreen(p: Point): Offset {
                        return Offset((p.x * finalScale + finalOffset.x).toFloat(),
                            (p.y * finalScale + finalOffset.y).toFloat())
                    }

                    for (point in initP)
                    {
                        val screenPoint = toScreen(point)
                        drawCircle(
                            color = Color.Blue,
                            radius = 5f,
                            center = screenPoint
                        )
                    }
                    for (line in initL) {
                        val p1: Point
                        val p2: Point
                        val a=line.a
                        val b=line.b
                        val c=line.c
                        if (abs(b) > 1e-8) {
                            val x1 = -size.width * 2f
                            val y1 = ((c - a * x1) / b)
                            val x2 = size.width * 2f
                            val y2 = ((c - a * x2) / b)
                            p1 = Point(x1.toDouble(), y1)
                            p2 = Point(x2.toDouble(), y2)
                        } else if (abs(a) > 1e-8) { // b 为 0，a 不为 0，说明是完全垂直的线

                            val x = (c / a).toFloat()
                            p1 = Point(x.toDouble(), (-size.height * 2f).toDouble())
                            p2 = Point(x.toDouble(), (size.height * 2f).toDouble())
                        } else {
                            continue
                        }


                        drawLine(
                            color = Color.Blue,
                            start = toScreen(p1),
                            end = toScreen(p2),
                            strokeWidth = 2f
                        )
                    }


                    for(circle in initC)
                    {
                        val a=circle.centerX
                        val b=circle.centerY
                        val r=circle.radius
                        val screenCenter = toScreen(Point(a, b))

                        val screenRadius = r.toFloat()

                        drawCircle(
                            color = Color.Blue,
                            radius = screenRadius*finalScale,
                            center = screenCenter,
                            style = Stroke(width = 2f)
                        )
                    }
                    for (i in 0..currentStepIndex+1) {
                        if (i >= drawSteps.size) break
                        val step = drawSteps[i]

                        when (step.stepIndex) {

                                0-> {

                                val center = step.p1
                                val pointOnCircle = step.p2

                                if (center != null && pointOnCircle != null) {
                                    val screenCenter = Offset(
                                        (center[0] * finalScale + finalOffset.x).toFloat(),
                                        (center[1] * finalScale + finalOffset.y).toFloat()
                                    )
                                    val radius = sqrt(
                                        (pointOnCircle[0] - center[0]).pow(2) + (pointOnCircle[1] - center[1]).pow(2)
                                    ) * finalScale

                                    drawCircle(
                                        color = Color.Green,
                                        radius = radius.toFloat(),
                                        center = screenCenter,
                                        style = Stroke(width = 2f)
                                    )
                                }
                            }
                            1 -> {

                                val center = step.p2
                                val pointOnCircle = step.p1

                                if (center != null && pointOnCircle != null) {
                                    val screenCenter = Offset(
                                        (center[0] * finalScale + finalOffset.x).toFloat(),
                                    (center[1] * finalScale + finalOffset.y).toFloat()
                                    )
                                    val radius = sqrt(
                                        (pointOnCircle[0] - center[0]).pow(2) + (pointOnCircle[1] - center[1]).pow(2)
                                    ) * finalScale

                                    drawCircle(
                                        color = Color.Green,
                                        radius = radius.toFloat(),
                                        center = screenCenter,
                                        style = Stroke(width = 2f)
                                    )
                                }
                            }

                            2-> {
                                val p1 = step.p1
                                val p2 = step.p2

                                if (p1 != null && p2 != null) {

                                    val sp1 = Offset((p1[0] * finalScale + finalOffset.x).toFloat(), (p1[1] * finalScale + finalOffset.y).toFloat())
                                    val sp2 = Offset((p2[0] * finalScale + finalOffset.x).toFloat(), (p2[1] * finalScale + finalOffset.y).toFloat())


                                    val dx = sp2.x - sp1.x
                                    val dy = sp2.y - sp1.y
                                    val length = sqrt(dx * dx + dy * dy)

                                    if (length > 0.001f) {
                                        val dirX = dx / length
                                        val dirY = dy / length

                                        val extendLength = max(size.width, size.height) * 2f

                                        val start = Offset(sp1.x - dirX * extendLength, sp1.y - dirY * extendLength)
                                        val end = Offset(sp2.x + dirX * extendLength, sp2.y + dirY * extendLength)

                                        drawLine(
                                            color = Color.Black,
                                            start = start,
                                            end = end,
                                            strokeWidth = 2f
                                        )
                                    }
                                }
                            }
                        }

                    }
                    if(currentStepIndex==drawSteps.size-1)
                    {
                        if(goalnote==0)
                        {
                            val screenPoint = toScreen(Point(targe[0],targe[1]))
                            drawCircle(
                                color = Color.Yellow,
                                radius = 5f,
                                center = screenPoint
                            )
                        }
                        else if(goalnote==1)
                        {
                            var p1=Point(0.0,0.0)
                            var p2=Point(0.0,0.0)
                            val a=targe[0]
                            val b=targe[1]
                            val c=targe[2]
                            var bo: Boolean=true
                            if (abs(b) > 1e-8) {
                                val x1 = -size.width * 2f
                                val y1 = ((c - a * x1) / b)
                                val x2 = size.width * 2f
                                val y2 = ((c - a * x2) / b)
                                p1 = Point(x1.toDouble(), y1)
                                p2 = Point(x2.toDouble(), y2)
                            } else if (abs(a) > 1e-8) {
                                val x = (c / a).toFloat()
                                p1 = Point(x.toDouble(), (-size.height * 2f).toDouble())
                                p2 = Point(x.toDouble(), (size.height * 2f).toDouble())
                            }
                            else
                                bo=false

                            if(bo){
                            drawLine(
                                color = Color.Yellow,
                                start = toScreen(p1),
                                end = toScreen(p2),
                                strokeWidth = 2f
                            )}
                        }
                        else if(goalnote==2)
                        {
                            val a=targe[0]
                            val b=targe[1]
                            val r=targe[2]
                            val screenCenter = toScreen(Point(a, b))

                            val screenRadius = r.toFloat()

                            drawCircle(
                                color = Color.Yellow,
                                radius = screenRadius*finalScale,
                                center = screenCenter,
                                style = Stroke(width = 2f)
                            )
                        }
                    }
                }


                LazyColumn(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp, vertical = 4.dp)
                ) {
                    itemsIndexed(drawSteps) { index, step ->

                        val stepText = when (step.stepIndex) {
                            0 -> "以点 P${step.pointIndex1} 为圆心，作过点 P${step.pointIndex2} 的圆"
                            1 -> "以点 P${step.pointIndex1} 为圆心，作过点 P${step.pointIndex2} 的圆"
                            2 -> "连接点 P${step.pointIndex1} 和点 P${step.pointIndex2}"
                            else -> "第 ${index + 1} 步：未知操作 (type=${step.stepIndex})"
                        }

                        Text(
                            text = "第 ${index + 1} 步：$stepText",
                            color = if (index <= currentStepIndex) Color.Black else Color.Gray,
                            fontSize = 14.sp,
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 4.dp)
                        )


                        Divider(color = Color.LightGray, thickness = 0.5.dp)
                    }
                }

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp, vertical = 4.dp),
                    horizontalArrangement = Arrangement.SpaceEvenly
                ) {

                    IconButton(onClick = {
                        isPlaying = false
                        currentStepIndex = 0
                    }) {
                        Icon(Icons.Default.Replay, contentDescription = "重置")
                    }


                    IconButton(
                        onClick = { if (currentStepIndex > 0) currentStepIndex-- },
                        enabled = currentStepIndex > 0
                    ) {
                        Icon(Icons.Default.SkipPrevious, contentDescription = "上一步")
                    }


                    IconButton(onClick = { isPlaying = !isPlaying }) {
                        Icon(
                            if (isPlaying) Icons.Default.Pause else Icons.Default.PlayArrow,
                            contentDescription = if (isPlaying) "暂停" else "播放"
                        )
                    }


                    IconButton(
                        onClick = { if (currentStepIndex < drawSteps.size - 1) currentStepIndex++ },
                        enabled = currentStepIndex < drawSteps.size - 1
                    ) {
                        Icon(Icons.Default.SkipNext, contentDescription = "下一步")
                    }
                }
        }}
        if (showDialog) {
            WizardConstructionDialog(
                currentMode = selectedMode,
                onDismiss = { showDialog = false },
                onFinish = { finSteps ->
                    // 接收求解结果
                    drawSteps = finSteps.stepResult
                    initP=finSteps.initP
                    initL=finSteps.initL
                    initC=finSteps.initC
                    goalnote=finSteps.goalmode
                    targe= when(goalnote)
                    {
                        0 -> finSteps.targetP
                        1 -> finSteps.targetL
                        2 -> finSteps.targetC
                        else -> doubleArrayOf()
                    }

                    currentStepIndex = if (finSteps.stepResult.isNotEmpty()) finSteps.stepResult.size - 1 else 0
                    showDialog = false
                }
            )

        }
    }
}

@Composable
fun WizardConstructionDialog(
    currentMode: String,
    onDismiss: () -> Unit,
    onFinish: (FinResult) -> Unit
) {
    var progressForUi by remember { mutableStateOf(0f) }
    var showSolutionDialog by remember { mutableStateOf(false) }
    var currentSolution by remember { mutableStateOf<Solution?>(null) }
    var errorMessage by remember { mutableStateOf<String?>(null) }
    var step by remember { mutableStateOf(WizardStep.STEP_LIMIT) }
    var limit by remember { mutableStateOf("5") }
    var targetType by remember { mutableStateOf("点") }
    var targetRow by remember { mutableStateOf(mutableListOf("", "")) }
    var searchProgress by remember { mutableStateOf(0f) }
    var condPointCount by remember { mutableStateOf("0") }
    var condPointData by remember { mutableStateOf(listOf<MutableList<String>>()) }
    var condLineCount by remember { mutableStateOf("0") }
    var condLineData by remember { mutableStateOf(listOf<MutableList<String>>()) }
    var condCircleCount by remember { mutableStateOf("0") }
    var condCircleData by remember { mutableStateOf(listOf<MutableList<String>>()) }
    // 计算状态
    var isCalculating by remember { mutableStateOf(false) }
    var calculationMessage by remember { mutableStateOf("") }
    var modeNote by remember { mutableStateOf(-1) }
    var goalNote by remember { mutableStateOf(-1) }
    val validTargetTypes = when (currentMode) {
        "单尺作图" -> listOf("点", "线")
        "单规作图" -> listOf("点", "圆")
        else -> listOf("点", "线", "圆")
    }

    fun initCondList(count: Int, fields: Int): List<MutableList<String>> {
        return List(count) { MutableList(fields) { "" } }
    }

    AlertDialog(
        onDismissRequest = {
            if (!isCalculating) {
                onDismiss()
            }
        },
        title = {
            Text(when (step) {
                WizardStep.STEP_LIMIT -> "第一步：步数限制"
                WizardStep.STEP_TARGET_TYPE -> "第二步：目标类型"
                WizardStep.STEP_TARGET_DATA -> "第三步：目标数据"
                WizardStep.STEP_COND_POINT_COUNT -> "第四步：已知点个数"
                WizardStep.STEP_COND_POINT_DATA -> "第五步：已知点数据"
                WizardStep.STEP_COND_LINE_COUNT -> "第六步：已知线个数"
                WizardStep.STEP_COND_LINE_DATA -> "第七步：已知线数据"
                WizardStep.STEP_COND_CIRCLE_COUNT -> "第八步：已知圆个数"
                WizardStep.STEP_COND_CIRCLE_DATA -> {

                    if ((condCircleCount.toIntOrNull() ?: 0) == 0) "第九步：确认并开始计算" else "第九步：已知圆数据"
                }
            })
        },
        text = {
            Column(modifier = Modifier.verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                if (isCalculating) {
                    Text(
                        "搜索进度: ${(searchProgress *100).toInt()}%",
                        fontWeight = FontWeight.Bold

                    )


                    LinearProgressIndicator(
                        progress = searchProgress,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(8.dp),
                    )

                    Text(calculationMessage, textAlign = TextAlign.Center)
                    //Text(calculationMessage, fontWeight = FontWeight.Bold)
                } else {
                    Column {

                        if (errorMessage != null) {
                            Text(errorMessage!!, color = MaterialTheme.colorScheme.error, fontWeight = FontWeight.Bold)
                            Spacer(modifier = Modifier.height(8.dp))
                        }
                        else if (calculationMessage.isNotEmpty()) {

                            val isError = calculationMessage.startsWith("❌")
                            Text(
                                text = calculationMessage,
                                color = if (isError) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.onSurface,
                                fontWeight = FontWeight.Bold
                            )
                            Spacer(modifier = Modifier.height(8.dp))
                        }
                        when (step) {
                            WizardStep.STEP_LIMIT -> OutlinedTextField(
                                limit,
                                { limit = it },
                                label = { Text("最大步数") },
                                singleLine = true,
                                modifier = Modifier.fillMaxWidth()
                            )

                            WizardStep.STEP_TARGET_TYPE -> {
                                Text("请选择目标元素类型：")
                                Row {
                                    validTargetTypes.forEach {
                                        FilterChip(
                                            selected = targetType == it,
                                            onClick = { targetType = it },
                                            label = { Text(it) }); Spacer(Modifier.width(8.dp))
                                    }
                                }
                            }

                            WizardStep.STEP_TARGET_DATA -> {
                                Text("输入目标元素数据", fontWeight = FontWeight.Bold)
                                DynamicInputRow(
                                    type = targetType,
                                    row = targetRow,
                                    onValueChange = { index, value ->
                                        val newRow = targetRow.toMutableList(); newRow[index] =
                                        value; targetRow = newRow
                                    })
                            }

                            WizardStep.STEP_COND_POINT_COUNT -> OutlinedTextField(
                                condPointCount,
                                { condPointCount = it },
                                label = { Text("已知点个数") },
                                singleLine = true,
                                modifier = Modifier.fillMaxWidth()
                            )

                            WizardStep.STEP_COND_POINT_DATA -> condPointData.forEachIndexed { i, row ->
                                Text("已知点 ${i + 1}", fontWeight = FontWeight.Bold)
                                DynamicInputRow(
                                    type = "点",
                                    row = row,
                                    onValueChange = { index, value ->
                                        condPointData = condPointData.mapIndexed { idx, r ->
                                            if (idx == i) {
                                                val nr = r.toMutableList(); nr[index] = value; nr
                                            } else r
                                        }
                                    })
                            }

                            WizardStep.STEP_COND_LINE_COUNT -> OutlinedTextField(
                                condLineCount,
                                { condLineCount = it },
                                label = { Text("已知线个数") },
                                singleLine = true,
                                modifier = Modifier.fillMaxWidth()
                            )

                            WizardStep.STEP_COND_LINE_DATA -> condLineData.forEachIndexed { i, row ->
                                Text("已知线 ${i + 1}", fontWeight = FontWeight.Bold)
                                DynamicInputRow(
                                    type = "线",
                                    row = row,
                                    onValueChange = { index, value ->
                                        condLineData = condLineData.mapIndexed { idx, r ->
                                            if (idx == i) {
                                                val nr = r.toMutableList(); nr[index] = value; nr
                                            } else r
                                        }
                                    })
                            }

                            WizardStep.STEP_COND_CIRCLE_COUNT -> OutlinedTextField(
                                condCircleCount,
                                { condCircleCount = it },
                                label = { Text("已知圆个数") },
                                singleLine = true,
                                modifier = Modifier.fillMaxWidth()
                            )

                            WizardStep.STEP_COND_CIRCLE_DATA -> condCircleData.forEachIndexed { i, row ->
                                Text("已知圆 ${i + 1}", fontWeight = FontWeight.Bold)
                                DynamicInputRow(
                                    type = "圆",
                                    row = row,
                                    onValueChange = { index, value ->
                                        condCircleData = condCircleData.mapIndexed { idx, r ->
                                            if (idx == i) {
                                                val nr = r.toMutableList(); nr[index] = value; nr
                                            } else r
                                        }
                                    })
                            }
                        }
                    }
                }
            }
        },
        confirmButton = {
            Button(
                onClick = {
                    when (step) {
                        WizardStep.STEP_LIMIT -> step = WizardStep.STEP_TARGET_TYPE
                        WizardStep.STEP_TARGET_TYPE -> {
//                            targetType=it
                            targetRow = MutableList(if (targetType == "点") 2 else 3) { "" };
                            goalNote = when (targetType) {
                                "点" -> 0
                                "线" -> 1
                                "圆" -> 2
                                else -> -1
                            }
                            step = WizardStep.STEP_TARGET_DATA
                        }
                        WizardStep.STEP_TARGET_DATA -> step = WizardStep.STEP_COND_POINT_COUNT
                        WizardStep.STEP_COND_POINT_COUNT -> { val c = condPointCount.toIntOrNull() ?: 0; condPointData = initCondList(c, 2); step = if (c > 0) WizardStep.STEP_COND_POINT_DATA else WizardStep.STEP_COND_LINE_COUNT }
                        WizardStep.STEP_COND_POINT_DATA -> step = WizardStep.STEP_COND_LINE_COUNT
                        WizardStep.STEP_COND_LINE_COUNT -> { val c = condLineCount.toIntOrNull() ?: 0; condLineData = initCondList(c, 3); step = if (c > 0) WizardStep.STEP_COND_LINE_DATA else WizardStep.STEP_COND_CIRCLE_COUNT }
                        WizardStep.STEP_COND_LINE_DATA -> step = WizardStep.STEP_COND_CIRCLE_COUNT
                        WizardStep.STEP_COND_CIRCLE_COUNT -> { val c = condCircleCount.toIntOrNull() ?: 0; condCircleData = initCondList(c, 3); step = if (c > 0) WizardStep.STEP_COND_CIRCLE_DATA else WizardStep.STEP_COND_CIRCLE_DATA }
                        WizardStep.STEP_COND_CIRCLE_DATA -> {

                            errorMessage = null


                            if (targetType == "点") {
                                if (targetRow[0].toDoubleOrNull() == null || targetRow[1].toDoubleOrNull() == null) {
                                    errorMessage = "目标点数据有误，请检查X和Y坐标是否为有效数字。"
                                }
                            } else if (targetType == "线") {
                                if (targetRow[0].toDoubleOrNull() == null || targetRow[1].toDoubleOrNull() == null || targetRow[2].toDoubleOrNull() == null) {
                                    errorMessage = "目标线数据有误，请检查A, B, C是否为有效数字。"
                                }
                            } else if (targetType == "圆") {
                                if (targetRow[0].toDoubleOrNull() == null || targetRow[1].toDoubleOrNull() == null || targetRow[2].toDoubleOrNull() == null) {
                                    errorMessage =
                                        "目标圆数据有误，请检查圆心坐标和半径是否为有效数字。"
                                }
                            }


                            if (errorMessage != null) return@Button

                            val initPoints = condPointData.mapNotNull { row ->
                                val x = row[0].toDoubleOrNull()
                                val y = row[1].toDoubleOrNull()
                                if (x != null && y != null) Point(x, y) else null
                            }

                            val initLines = condLineData.mapNotNull { row ->
                                val a = row[0].toDoubleOrNull()
                                val b = row[1].toDoubleOrNull()
                                val c = row[2].toDoubleOrNull()
                                if (a != null && b != null && c != null) Line(a, b, c) else null
                            }

                            val initCircles = condCircleData.mapNotNull { row ->
                                val a = row[0].toDoubleOrNull()
                                val b = row[1].toDoubleOrNull()
                                val r = row[2].toDoubleOrNull()
                                if (a != null && b != null && r != null) Circle(a, b, r) else null
                            }

                            val targetPoints = if (targetType == "点") {
                                val x = targetRow[0].toDoubleOrNull()
                                val y = targetRow[1].toDoubleOrNull()
                                if (x != null && y != null) listOf(Point(x, y)) else emptyList()
                            } else {
                                emptyList()
                            }

                            val targetLines = if (targetType == "线") {
                                val a = targetRow[0].toDoubleOrNull()
                                val b = targetRow[1].toDoubleOrNull()
                                val c = targetRow[2].toDoubleOrNull()
                                if (a != null && b != null && c != null) listOf(
                                    Line(
                                        a,
                                        b,
                                        c
                                    )
                                ) else emptyList()
                            } else {
                                emptyList()
                            }

                            val targetCircles = if (targetType == "圆") {
                                val a = targetRow[0].toDoubleOrNull()
                                val b = targetRow[1].toDoubleOrNull()
                                val r = targetRow[2].toDoubleOrNull()
                                if (a != null && b != null && r != null) listOf(
                                    Circle(
                                        a,
                                        b,
                                        r
                                    )
                                ) else emptyList()
                            } else {
                                emptyList()
                            }

                            modeNote = when (currentMode) {
                                "尺规作图" -> 0
                                "单尺作图" -> 1
                                "单规作图" -> 2
                                else -> -1
                            }
                            val stepLimit = limit.toIntOrNull() ?: 5


                            val targetPointsForCpp = if (targetType == "点") {                      //
                                val x = targetRow[0].toDoubleOrNull()                              //
                                val y = targetRow[1].toDoubleOrNull()                              //
                                if (x != null && y != null) listOf(Point(x, y)) else emptyList()   //
                            } else {                                                               //
                                emptyList()                                                        //
                            }
                            val targetLinesForCpp=if(targetType=="线"){
                                val a=targetRow[0].toDoubleOrNull()
                                val b=targetRow[1].toDoubleOrNull()
                                val c=targetRow[2].toDoubleOrNull()
                                if(a!=null && b!=null &&c!=null)listOf(Line(a,b,c)) else emptyList()
                            }
                            else emptyList()
                            val targetCirclesForCpp=if(targetType=="圆"){
                                val a=targetRow[0].toDoubleOrNull()
                                val b=targetRow[1].toDoubleOrNull()
                                val r=targetRow[2].toDoubleOrNull()
                                if(a!=null && b!=null &&r!=null)listOf(Circle(a,b,r)) else emptyList()
                            }
                            else emptyList()
                            isCalculating = true
                            calculationMessage = "正在暴力搜索解法，请稍候..."

                            kotlinx.coroutines.GlobalScope.launch(Dispatchers.Default) {
                                try {
                                    val cppResults = Solver().solveGeometryProblem(
                                        initPoints = pointsToDoubleArray(initPoints),
                                        initPointCount = initPoints.size,
                                        initLines = linesToDoubleArray(initLines),
                                        initLineCount = initLines.size,
                                        initCircles = circlesToDoubleArray(initCircles),
                                        initCircleCount = initCircles.size,
                                        targetPoints = pointsToDoubleArray(targetPointsForCpp),
                                        targetLine= linesToDoubleArray(targetLinesForCpp) ,
                                        targetCircle=circlesToDoubleArray(targetCirclesForCpp),
                                        mode = currentMode,
                                        stepLimit = stepLimit,
//                                        modeNote.also { modeNote = it },
//                                        var goalNote : kotlin . Any ? = goalNote
                                        modenote = modeNote,   // 新增参数
                                        goalNote = goalNote
                                    )

                                    withContext(Dispatchers.Main) {
                                        isCalculating = false
                                        if (cppResults != null && cppResults.isNotEmpty()) {

                                            val targetPointsForCpp1 = if (targetType == "点") {                      //
                                                doubleArrayOf(targetRow[0].toDouble(),
                                                    targetRow[1].toDouble())                             //
//                                                if (x != null && y != null) listOf(Point(x, y)) else emptyList()   //
                                            } else {                                                               //
                                                doubleArrayOf()                                                  //
                                            }
                                            val targetLinesForCpp1=if(targetType=="线"){
                                                doubleArrayOf(targetRow[0].toDouble(),
                                                    targetRow[1].toDouble(),
                                                    targetRow[2].toDouble())
//                                                if(a!=null && b!=null &&c!=null)listOf(Line(a,b,c)) else emptyList()
                                            }
                                            else doubleArrayOf()
                                            val targetCirclesForCpp1=if(targetType=="圆"){
                                                doubleArrayOf(targetRow[0].toDouble(),
                                                targetRow[1].toDouble(),
                                                targetRow[2].toDouble())
//                                                if(a!=null && b!=null &&r!=null)listOf(Circle(a,b,r)) else emptyList()
                                            }
                                            else doubleArrayOf()
//                                            var targe=listOf(targetRow[0].toDoubleOrNull(),targetRow[1].toDoubleOrNull(),targetRow[2].toDoubleOrNull())
                                            var finre=FinResult(cppResults,
                                            goalNote,
                                            targetPointsForCpp1,
                                                targetLinesForCpp1,
                                                targetCirclesForCpp1,
                                            initPoints,
                                            initLines,
                                            initCircles)
                                            onFinish(finre)

                                            onDismiss()

                                            //showSolutionDialog = true; currentSolution = convertToSolution(cppResults)
                                        } else {

                                            calculationMessage = "❌ 在 $stepLimit 步内未找到解法。"
                                        }
                                    }
                                } catch (e: Exception) {
                                    withContext(Dispatchers.Main) {
                                        isCalculating = false
                                        calculationMessage = "❌ 求解出错: ${e.message}"
                                        e.printStackTrace()
                                    }
                                }
                            }
                        }

                    }
                },
                enabled = !isCalculating
            ) {
                Text(when {
                    isCalculating -> "计算中..."
                    step == WizardStep.STEP_COND_CIRCLE_DATA -> "开始计算"
                    step in listOf(WizardStep.STEP_COND_POINT_COUNT, WizardStep.STEP_COND_LINE_COUNT, WizardStep.STEP_COND_CIRCLE_COUNT) -> {
                        val c = when(step) { WizardStep.STEP_COND_POINT_COUNT -> condPointCount; WizardStep.STEP_COND_LINE_COUNT -> condLineCount; else -> condCircleCount }.toIntOrNull() ?: 0
                        if (c > 0) "下一步" else "跳过"
                    }
                    else -> "下一步"
                })
            }
        },
        dismissButton = {
            Row {
                if (!isCalculating && step != WizardStep.STEP_LIMIT) {
                    TextButton(onClick = {
                        step = when (step) {
                            WizardStep.STEP_TARGET_TYPE -> WizardStep.STEP_LIMIT
                            WizardStep.STEP_TARGET_DATA -> WizardStep.STEP_TARGET_TYPE
                            WizardStep.STEP_COND_POINT_COUNT -> WizardStep.STEP_TARGET_DATA
                            WizardStep.STEP_COND_POINT_DATA -> WizardStep.STEP_COND_POINT_COUNT
                            WizardStep.STEP_COND_LINE_COUNT -> if ((condPointCount.toIntOrNull() ?: 0) > 0) WizardStep.STEP_COND_POINT_DATA else WizardStep.STEP_TARGET_DATA
                            WizardStep.STEP_COND_LINE_DATA -> WizardStep.STEP_COND_LINE_COUNT
                            WizardStep.STEP_COND_CIRCLE_COUNT -> if ((condLineCount.toIntOrNull() ?: 0) > 0) WizardStep.STEP_COND_LINE_DATA else if ((condPointCount.toIntOrNull() ?: 0) > 0) WizardStep.STEP_COND_POINT_DATA else WizardStep.STEP_TARGET_DATA
                            WizardStep.STEP_COND_CIRCLE_DATA -> WizardStep.STEP_COND_CIRCLE_COUNT
                            else -> WizardStep.STEP_LIMIT
                        }
                    }) { Text("上一步") }
                }
                TextButton(onClick = onDismiss, enabled = !isCalculating) { Text("取消") }
            }
        }
    )
    LaunchedEffect(progressForUi) {
        searchProgress = progressForUi
    }
    if (showSolutionDialog && currentSolution != null) {
        AlertDialog(
            onDismissRequest = { showSolutionDialog = false },
            title = { Text("求解成功！") },
            text = {
                Column(modifier = Modifier.verticalScroll(rememberScrollState())) {
                    Text("共找到 ${currentSolution!!.steps.size} 步解法：", fontWeight = FontWeight.Bold)
                    Spacer(modifier = Modifier.height(8.dp))
                    currentSolution!!.steps.forEachIndexed { index, step ->
                        Text("第 ${index + 1} 步: 使用工具 ${step.tool}，操作点索引 ${step.p1Index} 和 ${step.p2Index}")
                    }
                }
            },
            confirmButton = {
                Button(onClick = { showSolutionDialog = false }) {
                    Text("关闭")
                }
            }
        )
    }
}
@Composable
fun DynamicInputRow(
    type: String,
    row: List<String>,
    onValueChange: (Int, String) -> Unit
) {
    val labels = when (type) {
        "点" -> listOf("X 坐标", "Y 坐标")
        "线" -> listOf("A", "B", "C")
        "圆" -> listOf("圆心 X", "圆心 Y", "半径 R")
        else -> listOf("值")
    }

    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        labels.forEachIndexed { index, label ->
            OutlinedTextField(
                value = row.getOrElse(index) { "" },
                onValueChange = { newValue -> onValueChange(index, newValue) },
                label = { Text(label, style = MaterialTheme.typography.bodySmall) },
                singleLine = true,
                modifier = Modifier.weight(1f)
            )
        }
    }
}

fun pointsToDoubleArray(points: List<Point>): DoubleArray {
    val result = DoubleArray(points.size * 2)
    points.forEachIndexed { index, point ->
        result[index * 2] = point.x
        result[index * 2 + 1] = point.y
    }
    return result
}

fun linesToDoubleArray(lines: List<Line>): DoubleArray {
    val result = DoubleArray(lines.size * 3)
    lines.forEachIndexed { index, line ->
        result[index * 3] = line.a
        result[index * 3 + 1] = line.b
        result[index * 3 + 2] = line.c
    }
    return result
}

fun circlesToDoubleArray(circles: List<Circle>): DoubleArray {
    val result = DoubleArray(circles.size * 3)
    circles.forEachIndexed { index, circle ->
        result[index * 3] = circle.centerX
        result[index * 3 + 1] = circle.centerY
        result[index * 3 + 2] = circle.radius
    }
    return result
}
